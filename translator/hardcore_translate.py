#!/usr/bin/env python3
"""Hardcore translator — deepseek-v4-pro + reverser_hardcore.md + multi-turn.

For routines that the standard P3 v2 pipeline (gemma4:31b + multi-pass critic)
cannot crack. Uses deepseek-v4-pro on Ollama Cloud with a much richer system
prompt (reverser_hardcore.md, 624 lines) and an iterative-refinement loop
that imitates a human operator pushing the LLM through repeated failures.

Multi-turn flow per routine (default 3 turns):

  Turn 1: standard translate. Validate via auto-spike.
  Turn 2: if FAIL, append verbatim failure tail to the conversation and ask
          for a corrected full C body. Re-validate.
  Turn 3: if still FAIL, append the new failure tail and retry once more.

Custom_spike and pass/delegate_pass exit immediately (no point retrying when
the oracle won't help or we already succeeded).

Auto-prepends `#include "snes/snes.h"` to every emitted .c (deepseek
systematically forgets it under the hardcore prompt).

Usage:
    python translator/hardcore_translate.py \\
        --names battle:Special_06 field:WaitSpecial menu:UpdateCtrl \\
        --max-turns 3 \\
        --out-dir port/_hardcore
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import urllib.request
from pathlib import Path

THIS = Path(__file__).resolve().parent
ROOT = THIS.parent
sys.path.insert(0, str(THIS))
import batch_translate as bt  # type: ignore

DEFAULT_MODEL = "deepseek-v4-pro"
DEFAULT_HARDCORE_PROMPT = ROOT / "prompts" / "reverser_hardcore.md"
DEFAULT_TASK_TEMPLATE = ROOT / "prompts" / "reverser_task.md"
DEFAULT_EXAMPLES = ROOT / "prompts" / "reverser_examples.md"
DEFAULT_API_BASE = "https://ollama.com/v1"
DEFAULT_KEY_PATH = Path.home() / ".ollama" / "ff4-port.api.key"
GENERATE_SPIKE = THIS / "generate_spike.py"


# ─────────────────────────────────────────────────────────────────────
# Chat completions + code extraction
# ─────────────────────────────────────────────────────────────────────

def call_chat(messages: list[dict], model: str, api_base: str, api_key: str,
                max_output_tokens: int, temperature: float = 0.0,
                timeout: int = 300) -> dict:
    """POST to /chat/completions with a full messages history. Returns:
      {text, reasoning, finish_reason, tokens_in, tokens_out, raw_msg, error}
    """
    payload = {
        "model": model,
        "messages": messages,
        "max_tokens": max_output_tokens,
        "temperature": temperature,
    }
    req = urllib.request.Request(
        f"{api_base.rstrip('/')}/chat/completions",
        data=json.dumps(payload).encode(),
        headers={
            "Content-Type": "application/json",
            "Authorization": f"Bearer {api_key}",
        },
    )
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            data = json.loads(resp.read().decode())
    except Exception as e:
        return {"text": "", "reasoning": "", "finish_reason": "",
                "tokens_in": 0, "tokens_out": 0, "raw_msg": "",
                "error": f"HTTP: {e}"}

    try:
        msg = data["choices"][0]["message"]
        text = msg.get("content") or ""
        reasoning = msg.get("reasoning") or ""
        finish = data["choices"][0].get("finish_reason", "")
        usage = data.get("usage", {})
        return {
            "text": text, "reasoning": reasoning, "finish_reason": finish,
            "tokens_in": usage.get("prompt_tokens", 0),
            "tokens_out": usage.get("completion_tokens", 0),
            "raw_msg": text or reasoning,
            "error": "",
        }
    except (KeyError, IndexError):
        return {"text": "", "reasoning": "", "finish_reason": "",
                "tokens_in": 0, "tokens_out": 0, "raw_msg": "",
                "error": "unexpected response shape"}


def extract_c_code(text: str) -> str | None:
    """Extract the C source from a fenced ```c ... ``` block. Returns
    None if no plausible C body is found."""
    if not text:
        return None
    m = re.search(r"```(?:c|cpp|C)?\s*\n(.*?)```", text, re.DOTALL)
    if m:
        body = m.group(1).strip()
        if body:
            return body
    # Heuristic fallback: if the text contains `void <name>_c(`, treat
    # everything from there to the trailing REVERSED_FUNCTION line as code.
    m = re.search(r"(void\s+\w+_c\s*\(.*?REVERSED_FUNCTION[^\n]*)",
                  text, re.DOTALL)
    if m:
        return m.group(1).strip()
    return None


def post_process_code(code: str) -> str:
    """Apply uniform post-processing: strip static, prepend snes/snes.h
    include if missing."""
    code = re.sub(r"^\s*static\s+(?=void\s+\w+_c\s*\()", "", code,
                   count=1, flags=re.MULTILINE)
    if '#include "snes/snes.h"' not in code:
        code = '#include "snes/snes.h"\n\n' + code
    return code


# ─────────────────────────────────────────────────────────────────────
# Validation (auto-spike harness)
# ─────────────────────────────────────────────────────────────────────

def validate_one(c_path: Path, tag: str, timeout: int = 90) -> dict:
    try:
        proc = subprocess.run(
            [sys.executable, str(GENERATE_SPIKE),
             str(c_path), "--build", "--run", "100",
             "--spike-suffix", tag],
            capture_output=True, text=True, cwd=str(ROOT), timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return {"status": "spike_timeout", "tail": ""}
    out = proc.stdout + "\n--- stderr ---\n" + proc.stderr
    m = re.search(r"=== summary === trials: (\d+), fails: (\d+)", out)
    if m and int(m.group(2)) == 0:
        return {"status": "pass", "trials": int(m.group(1)), "tail": ""}
    if m:
        return {"status": "ram_diverge", "trials": int(m.group(1)),
                "fails": int(m.group(2)), "tail": out[-1500:]}
    if "delegate wrapper" in out:
        return {"status": "delegate_pass", "tail": ""}
    if "skipping auto-gen" in out or "CUSTOM_SPIKE" in out:
        return {"status": "custom_spike", "tail": out[-600:]}
    if "implicit-function-declaration" in out or "undeclared" in out:
        return {"status": "compile_error", "tail": out[-2000:]}
    if "make: ***" in out or "Error 1" in out:
        return {"status": "compile_error", "tail": out[-2000:]}
    return {"status": "unknown", "tail": out[-1500:]}


# ─────────────────────────────────────────────────────────────────────
# Multi-turn translate
# ─────────────────────────────────────────────────────────────────────

# A status is "terminal" if there is no point re-asking the LLM.
TERMINAL_STATUSES = {"pass", "delegate_pass", "custom_spike", "spike_timeout"}


def translate_one_multiturn(
    mod: str, name: str, system_md: str, examples_md: str,
    task_template: str, out_dir: Path,
    model: str, api_base: str, api_key: str,
    max_output_tokens: int, max_turns: int = 3,
) -> dict:
    """Iterative-refinement translate: ask, validate, on failure feed the
    verbatim error back to the model and ask for a correction. Stop on
    pass / delegate_pass / custom_spike / max_turns."""
    out_dir = out_dir / mod
    out_dir.mkdir(parents=True, exist_ok=True)
    target_c = out_dir / f"{name}.c"

    # Hydrate the routine: fetch asm + xrefs from ca65-bridge.
    routine = bt.RoutineInfo(
        name=name, module=mod, address="", instr_count=0, call_count=0,
        decision="translate", reasons=["hardcore"],
        xrefs_out=[], asm_body="",
    )
    try:
        routine = bt.hydrate(routine)
    except Exception as e:
        return {"mod": mod, "name": name, "status": "hydrate_error",
                "error": str(e)[:300], "turns": []}
    if not routine.asm_body or len(routine.asm_body) < 30:
        return {"mod": mod, "name": name, "status": "asm_empty",
                "address": routine.address, "turns": []}

    user_prompt = bt.build_user_prompt(routine, "translate", task_template)
    combined_system = f"{system_md}\n\n# Reference examples\n\n{examples_md}"

    messages: list[dict] = [
        {"role": "system", "content": combined_system},
        {"role": "user",   "content": user_prompt},
    ]

    turns_history: list[dict] = []
    last_result = {"status": "no_code", "tail": ""}
    total_tokens_out = 0

    for turn in range(1, max_turns + 1):
        resp = call_chat(messages, model, api_base, api_key,
                         max_output_tokens, temperature=0.0)
        total_tokens_out += resp["tokens_out"]
        if resp["error"]:
            turns_history.append({
                "turn": turn, "status": f"http:{resp['error'][:80]}",
                "tokens_out": resp["tokens_out"],
            })
            last_result = {"status": "http_error", "tail": resp["error"]}
            break

        code = extract_c_code(resp["text"]) or extract_c_code(resp["reasoning"])
        if not code:
            turns_history.append({
                "turn": turn, "status": "no_code",
                "tokens_out": resp["tokens_out"],
                "finish": resp["finish_reason"],
            })
            last_result = {"status": "no_code", "tail": ""}
            # No code to correct — break (can't usefully retry an empty answer).
            break

        code = post_process_code(code)
        target_c.write_text(code)

        v = validate_one(target_c,
                          tag=f"hcm{turn}_{name.lower()[:18]}",
                          timeout=90)
        turns_history.append({
            "turn": turn, "status": v["status"],
            "tokens_out": resp["tokens_out"],
            "finish": resp["finish_reason"],
        })
        last_result = v
        sys.stderr.write(f"    turn {turn}: {v['status']} "
                         f"(tok_out={resp['tokens_out']})\n")

        if v["status"] in TERMINAL_STATUSES:
            break

        # We're going to retry. Append the assistant's previous reply
        # then the corrective user message with the verbatim error tail.
        if turn >= max_turns:
            break
        messages.append({"role": "assistant",
                          "content": resp["raw_msg"][:8000]})
        err_excerpt = (v.get("tail") or "")[-2500:]
        retry_msg = f"""Your previous translation FAILED validation.

Verdict: `{v['status']}`

Build / validator output (verbatim — focus on the FIRST `error:` line):

```
{err_excerpt}
```

Analyse the root cause precisely, then emit a COMPLETE corrected C body
for `{name}_c`. Constraints recap:
  - Signature exactly `void {name}_c(Snes *snes)` (no static, no inline).
  - Use ONLY the API names listed in H1 (no `snes->reg[]`, no
    `snes->memory[]`, no `jsl_long`, no `cpu_set_flag`).
  - Include `#include \"snes/snes.h\"` at the top.
  - If you cannot translate faithfully, emit a `delegate` wrapper per H4
    rather than guessing.
  - Output ONLY the new C code in a single ```c fenced block."""
        messages.append({"role": "user", "content": retry_msg})

    return {
        "mod": mod, "name": name,
        "status": last_result["status"],
        "c_path": str(target_c) if target_c.exists() else "",
        "turns": turns_history,
        "tokens_out_total": total_tokens_out,
    }


# ─────────────────────────────────────────────────────────────────────
# CLI driver
# ─────────────────────────────────────────────────────────────────────

def main():
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--names", nargs="+", help="mod:name entries")
    g.add_argument("--names-file", type=Path,
                   help="one mod:name per line (# comments and blanks ok)")
    ap.add_argument("--model", default=DEFAULT_MODEL)
    ap.add_argument("--system-md", type=Path, default=DEFAULT_HARDCORE_PROMPT)
    ap.add_argument("--task-md", type=Path, default=DEFAULT_TASK_TEMPLATE)
    ap.add_argument("--examples-md", type=Path, default=DEFAULT_EXAMPLES)
    ap.add_argument("--out-dir", type=Path, default=ROOT / "port" / "_hardcore")
    ap.add_argument("--api-base", default=DEFAULT_API_BASE)
    ap.add_argument("--api-key", default=None)
    ap.add_argument("--max-output-tokens", type=int, default=16384)
    ap.add_argument("--max-turns", type=int, default=3,
                    help="how many translate-validate cycles per routine "
                         "(1 = single-shot; 3 = up to 2 retries with "
                         "verbatim error feedback)")
    ap.add_argument("--log", type=Path,
                    default=THIS / "runs" / "hardcore_log.jsonl")
    args = ap.parse_args()

    api_key = args.api_key
    if api_key is None and DEFAULT_KEY_PATH.exists():
        api_key = DEFAULT_KEY_PATH.read_text().strip()
    if not api_key:
        sys.exit("no api key (--api-key or ~/.ollama/ff4-port.api.key)")

    if args.names:
        raw = args.names
    else:
        raw = [l.strip() for l in args.names_file.read_text().splitlines()]
    targets: list[tuple[str, str]] = []
    for line in raw:
        if not line or line.startswith("#") or ":" not in line:
            continue
        mod, name = line.split(":", 1)
        targets.append((mod.strip(), name.strip()))

    sys.stderr.write(f"[hardcore] {len(targets)} routines, max-turns={args.max_turns}, "
                     f"model={args.model}\n")
    system_md = args.system_md.read_text()
    examples_md = args.examples_md.read_text() if args.examples_md.exists() else ""
    task_template = args.task_md.read_text()
    sys.stderr.write(f"[hardcore] system_md {len(system_md)} bytes, "
                     f"examples {len(examples_md)} bytes\n")

    args.log.parent.mkdir(parents=True, exist_ok=True)
    log_fp = args.log.open("a")
    summary = {"pass": 0, "delegate_pass": 0, "ram_diverge": 0,
               "compile_error": 0, "custom_spike": 0, "no_code": 0,
               "unknown": 0, "spike_timeout": 0, "asm_empty": 0,
               "hydrate_error": 0, "http_error": 0}
    turns_by_pass = {1: 0, 2: 0, 3: 0}

    for i, (mod, name) in enumerate(targets, 1):
        sys.stderr.write(f"\n[{i}/{len(targets)}] {mod}:{name}\n")
        sys.stderr.flush()
        r = translate_one_multiturn(
            mod, name, system_md, examples_md, task_template,
            args.out_dir, args.model, args.api_base, api_key,
            args.max_output_tokens, max_turns=args.max_turns,
        )
        summary[r["status"]] = summary.get(r["status"], 0) + 1
        if r["status"] == "pass" and r["turns"]:
            last_turn_n = r["turns"][-1]["turn"]
            turns_by_pass[last_turn_n] = turns_by_pass.get(last_turn_n, 0) + 1
        rec = {"i": i, "mod": mod, "name": name,
               "final_status": r["status"],
               "c_path": r.get("c_path", ""),
               "turns": r.get("turns", []),
               "tokens_out_total": r.get("tokens_out_total", 0)}
        log_fp.write(json.dumps(rec) + "\n")
        log_fp.flush()

    log_fp.close()
    sys.stderr.write(f"\n[hardcore] summary: {summary}\n")
    sys.stderr.write(f"[hardcore] PASS came at turn: {turns_by_pass}\n")
    print(json.dumps({"summary": summary, "pass_by_turn": turns_by_pass},
                       indent=2))


if __name__ == "__main__":
    main()
