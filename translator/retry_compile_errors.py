#!/usr/bin/env python3
"""Phase 4.10 — retry compile_error translations with cc1 feedback.

INPUT:
  - A validation.jsonl produced by translator/validate_run.py
  - The corresponding port/_runs/<model>/<module>/<name>.c files

ALGORITHM:
  For each record with status == "compile_error":
    1. Read the previous C from `file`.
    2. Hydrate the asm body via ca65-bridge (single source of truth).
    3. Re-prompt the LLM with the standard task template PLUS a
       FEEDBACK section containing the previous C and the compiler's
       error_message.
    4. Overwrite `file` with the new C (if the LLM emitted code).
  One retry per routine — no exponential effort.

OUTPUT:
  - JSONL audit trail (translator/runs/<tag>_retry.jsonl by default)
  - Modified .c files in place. Use git to inspect/revert.

USAGE:
    OPENAI_API_KEY="$(cat ~/.ollama/ff4-port.api.key)" \\
      python translator/retry_compile_errors.py \\
        --validation translator/runs/qwen3_validation.jsonl \\
        --llm openai-compat --api-base https://ollama.com/v1 \\
        --model qwen3-coder:480b \\
        --tag qwen3 \\
        --max-output-tokens 3000

After the retry, re-run validate_run.py to measure the delta.
"""
from __future__ import annotations

import argparse
import json
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent

sys.path.insert(0, str(HERE))
from batch_translate import (  # noqa: E402  (sibling module, intentional)
    RoutineInfo,
    hydrate,
    load_prompts,
    build_user_prompt,
)
from llm_providers import create_provider, DEFAULT_MODELS  # noqa: E402


def load_compile_errors(validation_jsonl: Path) -> list[dict]:
    """Return the records with status=compile_error from a validation.jsonl."""
    out: list[dict] = []
    with validation_jsonl.open() as fp:
        for line in fp:
            line = line.strip()
            if not line:
                continue
            rec = json.loads(line)
            if rec.get("status") == "compile_error":
                out.append(rec)
    return out


def infer_module(rec: dict) -> str:
    """Pull the module name out of a path like port/_runs/<model>/<module>/<name>.c."""
    parts = Path(rec["file"]).parts
    # port/_runs/<model>/<module>/<name>.c
    if len(parts) >= 5 and parts[0] == "port" and parts[1] == "_runs":
        return parts[3]
    # Fallback: assume battle
    return "battle"


FEEDBACK_SUFFIX = """\

---

# PREVIOUS TRANSLATION ATTEMPT (failed C compilation)

```c
{prev_c}
```

# COMPILER ERROR (first line from cc1)

{error_msg}

# RETRY INSTRUCTIONS

The previous translation may match the asm semantically but failed to
compile. Fix the compile error WITHOUT changing the parity-equivalent
logic.

## HELPER NAMING — STRICT

Every `jsr Foo` / `jsl Foo` / `jsl Bank::Foo` in the asm becomes
`Foo_emu(snes);` in C. The helper MUST keep the EXACT PascalCase
identifier from the asm — NO snake_case rewriting, NO descriptive
renaming. If the asm calls `GetAICondTarget`, the C MUST call
`GetAICondTarget_emu(snes)`, never `get_ai_cond_target_emu` and never
`get_target`.

Canonical helper names for this routine (derived from xrefs_out):
{canonical_helpers}

Every `*_emu` helper takes exactly `(Snes *snes)` and returns void.
Do not pass extra arguments.

## OTHER COMMON CAUSES

  - Missing `cpu_*` helpers: use the names listed in the API reference
    section of the system prompt — do not invent variants.
  - Type/cast errors: `cpu_read*` / `cpu_write*` take `uint32_t`
    addresses, not pointers.
  - The reversed function MUST be declared as
    `static void <FunctionName>_c(Snes *snes)` — that exact name is
    what the spike harness calls. Do not emit a `static const` table
    when a function is expected: if the asm is purely tabular data,
    say so in a CONTRACT block and emit the table — the harness will
    skip you as CUSTOM_SPIKE.

Output the corrected C following the format in `reverser_system.md`.
Do not include the FEEDBACK or original asm — only the final C body
inside the standard fenced block.
"""


def _is_data_table(prev_c: str, routine_name: str) -> bool:
    """Heuristic: the previous attempt declared a table, not a function.

    True iff the code contains `static const` (typical table form) AND
    does NOT contain a `<Name>_c(Snes *snes)` function signature.
    """
    if "static const" not in prev_c:
        return False
    sig = f"{routine_name}_c(Snes"
    return sig not in prev_c


def _canonical_helper_names(xrefs_out: list[str]) -> str:
    """Extract just the helper PascalCase names from the xrefs_out lines.

    Each xrefs_out entry has the shape `Name @addr [type]`. We strip
    the trailing metadata, strip any `Bank::` prefix, and produce a
    bulleted list of expected `Name_emu(snes)` calls.
    """
    names: list[str] = []
    for x in xrefs_out:
        head = x.split(" @", 1)[0]
        head = head.rsplit("::", 1)[-1]
        if head:
            names.append(head)
    if not names:
        return "  (none — this routine calls no sub-routines)"
    seen: set[str] = set()
    out_lines: list[str] = []
    for n in names:
        if n in seen:
            continue
        seen.add(n)
        out_lines.append(f"  - {n}_emu(snes)")
    return "\n".join(out_lines)


def build_retry_prompt(
    routine: RoutineInfo,
    prev_c: str,
    error_msg: str,
    task_template: str,
) -> str:
    base = build_user_prompt(routine, "translate", task_template)
    canonical_helpers = _canonical_helper_names(routine.xrefs_out)
    return base + FEEDBACK_SUFFIX.format(
        prev_c=prev_c.strip(),
        error_msg=error_msg,
        canonical_helpers=canonical_helpers,
    )


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    ap.add_argument("--validation", type=Path, required=True,
                    help="Path to validate_run output (jsonl)")
    ap.add_argument("--llm", choices=["claude-cli", "anthropic-sdk", "openai-compat"],
                    default="openai-compat")
    ap.add_argument("--model", default=None,
                    help="Model name (per-provider default if unset)")
    ap.add_argument("--api-base", default=None,
                    help="Base URL for OpenAI-compat backends")
    ap.add_argument("--api-key", default=None)
    ap.add_argument("--claude-bin", default="claude")
    ap.add_argument("--max-output-tokens", type=int, default=3000)
    ap.add_argument("--max-functions", type=int, default=0,
                    help="Stop after N retries (0 = no limit)")
    ap.add_argument("--tag", required=True,
                    help="Run tag used to name the audit jsonl")
    ap.add_argument("--out-jsonl", type=Path, default=None,
                    help="Audit JSONL (default: translator/runs/<tag>_retry.jsonl)")
    ap.add_argument("--dry-run", action="store_true",
                    help="Print the prompt for the first record, exit without LLM call")
    ap.add_argument("--only", default=None,
                    help="Comma-separated list of routine names to process")
    args = ap.parse_args(argv)

    model = args.model or DEFAULT_MODELS[args.llm]
    provider = create_provider(
        args.llm,
        bin_path=args.claude_bin,
        api_base=args.api_base,
        api_key=args.api_key or os.environ.get("OPENAI_API_KEY"),
    )

    prompts = load_prompts()
    records = load_compile_errors(args.validation)
    sys.stderr.write(
        f"[retry] {args.validation.name}: {len(records)} compile_error rows\n"
    )

    only_set: set[str] | None = None
    if args.only:
        only_set = {s.strip() for s in args.only.split(",") if s.strip()}
        sys.stderr.write(f"[retry] --only filter: {sorted(only_set)}\n")

    out_jsonl = args.out_jsonl or HERE / "runs" / f"{args.tag}_retry.jsonl"
    out_jsonl.parent.mkdir(exist_ok=True, parents=True)
    log_fp = out_jsonl.open("a")

    n_done = 0
    n_rewritten = 0
    n_no_code = 0
    n_errors = 0

    for rec in records:
        name = rec["name"]
        if only_set is not None and name not in only_set:
            continue
        if args.max_functions and n_done >= args.max_functions:
            sys.stderr.write(f"[retry] reached --max-functions={args.max_functions}\n")
            break

        module = infer_module(rec)
        c_path = ROOT / rec["file"]
        if not c_path.is_file():
            sys.stderr.write(f"[retry] {name}: prev C missing at {c_path} — skipping\n")
            n_errors += 1
            continue

        prev_c = c_path.read_text()
        error_msg = rec.get("error_message") or "(no error_message captured)"

        if _is_data_table(prev_c, name):
            out_record = {
                "name": name,
                "module": infer_module(rec),
                "file": str(c_path.relative_to(ROOT)),
                "prev_error": error_msg,
                "status": "data_table_skip",
            }
            log_fp.write(json.dumps(out_record) + "\n")
            log_fp.flush()
            print(json.dumps(out_record), flush=True)
            n_done += 1
            continue

        r = RoutineInfo(
            name=name, module=module, address="",
            instr_count=0, call_count=0,
            decision="translate", reasons=[],
        )
        try:
            r = hydrate(r)
        except Exception as exc:
            sys.stderr.write(f"[retry] {name}: ca65-bridge hydrate failed: {exc}\n")
            n_errors += 1
            continue

        user_prompt = build_retry_prompt(r, prev_c, error_msg, prompts["task_template"])

        if args.dry_run:
            print(user_prompt)
            sys.stderr.write("[retry] --dry-run: printed prompt for first record, exiting\n")
            return 0

        code, stats = provider.translate(
            system=prompts["system"],
            examples=prompts["examples"],
            user_prompt=user_prompt,
            model=model,
            max_output_tokens=args.max_output_tokens,
            dry_run=False,
        )

        out_record = {
            "name": name,
            "module": module,
            "file": str(c_path.relative_to(ROOT)),
            "prev_error": error_msg,
            "provider": stats.provider,
            "model": stats.model,
            "tokens_in": stats.tokens_in,
            "tokens_out": stats.tokens_out,
            "cost_usd": round(stats.cost_usd, 4),
        }
        if stats.error:
            out_record["error"] = stats.error
            out_record["status"] = "llm_error"
            n_errors += 1
        elif not code:
            out_record["status"] = "no_code_extracted"
            n_no_code += 1
        else:
            c_path.write_text(code)
            out_record["status"] = "rewritten"
            out_record["bytes"] = len(code)
            n_rewritten += 1

        log_fp.write(json.dumps(out_record) + "\n")
        log_fp.flush()
        print(json.dumps(out_record), flush=True)
        n_done += 1

    log_fp.close()
    sys.stderr.write("\n[retry] === summary ===\n")
    sys.stderr.write(f"[retry] provider:    {args.llm} ({model})\n")
    sys.stderr.write(f"[retry] processed:   {n_done}\n")
    sys.stderr.write(f"[retry] rewritten:   {n_rewritten}\n")
    sys.stderr.write(f"[retry] no_code:     {n_no_code}\n")
    sys.stderr.write(f"[retry] errors:      {n_errors}\n")
    sys.stderr.write(f"[retry] audit log:   {out_jsonl}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
