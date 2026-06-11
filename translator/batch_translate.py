#!/usr/bin/env python3
"""FF4 batch translator — asm 65816 → C via pluggable LLM provider.

ARCHITECTURE:
1. Enumerate routines via ca65-bridge (filtered by module).
2. For each routine, classify_routine() → 'translate' | 'delegate'.
3. If delegate: emit a trivial C wrapper locally (zero LLM cost).
4. If translate: call an LLM through the chosen provider (--llm).
5. Track tokens consumed and $USD cost per call.
6. Stop automatically when the budget cap is exceeded.
7. Output: C code in port/<module>/<func>.c plus a JSONL log.

SUPPORTED LLM PROVIDERS (--llm):
  - claude-cli     : non-interactive `claude --print` (DEFAULT, Claude Code subscription)
  - anthropic-sdk  : Python anthropic SDK (pay-as-you-go API)
  - openai-compat  : OpenAI Chat Completions compatible (Ollama, OpenRouter, etc.)

USAGE (dry-run with claude CLI):
    python translator/batch_translate.py \\
        --module battle --max-functions 5 --dry-run

USAGE (real run with claude CLI, no budget cap = subscription):
    python translator/batch_translate.py \\
        --module battle --max-functions 3

USAGE (Anthropic SDK, hard cap $0.50):
    ANTHROPIC_API_KEY=$KEY python translator/batch_translate.py \\
        --module battle --max-functions 3 \\
        --llm anthropic-sdk --budget-usd 0.50

USAGE (local Ollama, free):
    python translator/batch_translate.py \\
        --module battle --max-functions 3 \\
        --llm openai-compat --api-base http://localhost:11434/v1 \\
        --model llama3:8b

Strict non-interactive mode: JSON on stdout, no questions asked.
"""
from __future__ import annotations

import argparse
import dataclasses
import json
import os
import re
import sys
from pathlib import Path
from typing import Optional

# ca65-bridge is installed in its own venv; we invoke it via subprocess to
# avoid creating a cross-package Python dependency.
import subprocess

# LLM providers (claude-cli, anthropic-sdk, openai-compat)
sys.path.insert(0, str(Path(__file__).resolve().parent))
from llm_providers import create_provider, CallStats, DEFAULT_MODELS


HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
UPSTREAM = ROOT / "upstream"
PROMPTS_DIR = ROOT / "prompts"
PORT_DIR = ROOT / "port"
LOG_FILE = HERE / "batch_log.jsonl"

DEFAULT_BUDGET = 0.0   # 0 = no API cost (claude CLI / local Ollama)


def _bridge(*args: str, cwd: Path = ROOT) -> str:
    """Invoke the ca65-bridge CLI installed in its local venv."""
    bridge_bin = ROOT / "ca65-bridge" / ".venv" / "bin" / "ca65-bridge"
    cmd = [str(bridge_bin), "--root", str(UPSTREAM)] + list(args)
    res = subprocess.run(cmd, capture_output=True, text=True, cwd=cwd)
    if res.returncode != 0:
        raise RuntimeError(f"ca65-bridge {args} failed:\n{res.stderr}")
    return res.stdout


@dataclasses.dataclass
class RoutineInfo:
    name: str
    module: str
    address: str        # e.g. "c987" (4-char hint)
    instr_count: int
    call_count: int
    decision: str       # 'translate' | 'delegate'
    reasons: list[str]
    asm_body: str = ""
    xrefs_out: list[str] = dataclasses.field(default_factory=list)


def enumerate_module(module: str) -> list[RoutineInfo]:
    """List the routines of a module with their classification."""
    out = _bridge("classify-module", module).strip().splitlines()
    routines: list[RoutineInfo] = []
    for line in out:
        m = re.match(r"^(\S+)\s+(translate|delegate)\s*(.*)$", line)
        if not m:
            continue
        name, decision, reasons_str = m.group(1, 2, 3)
        reasons = [r.strip() for r in reasons_str.split(";") if r.strip()]
        routines.append(RoutineInfo(
            name=name, module=module, address="",
            instr_count=0, call_count=0,
            decision=decision, reasons=reasons,
        ))
    return routines


def hydrate(r: RoutineInfo) -> RoutineInfo:
    """Load asm body + xrefs_out for a routine, ready to translate."""
    asm_text = _bridge("get-asm", r.name)
    # Parse header `# address_hint: XXXX  instr=N  calls=N` then body
    lines = asm_text.splitlines()
    header_re = re.match(r"^# address_hint:\s*(\S+)\s+instr=(\d+)\s+calls=(\d+)", lines[0])
    if header_re:
        r.address = header_re.group(1)
        r.instr_count = int(header_re.group(2))
        r.call_count = int(header_re.group(3))
    r.asm_body = "\n".join(lines[1:])

    xrefs_text = _bridge("xrefs-from", r.name)
    r.xrefs_out = []
    for line in xrefs_text.splitlines():
        m = re.match(r"^(\S+)\s+(\S+)\s+@(\S+)", line)
        if m:
            r.xrefs_out.append(f"{m.group(2)} @{m.group(3)} [{m.group(1)}]")
    return r


# ---------------------------------------------------------------------------
# Prompt assembly
# ---------------------------------------------------------------------------

def load_prompts() -> dict[str, str]:
    return {
        "system": (PROMPTS_DIR / "reverser_system.md").read_text(),
        "examples": (PROMPTS_DIR / "reverser_examples.md").read_text(),
        "task_template": (PROMPTS_DIR / "reverser_task.md").read_text(),
    }


def build_user_prompt(r: RoutineInfo, mode: str, task_template: str) -> str:
    """Render the task template with runtime values."""
    bank = r.address[:2] if len(r.address) >= 4 else "03"
    offset = r.address[2:] if len(r.address) >= 4 else r.address
    xrefs_str = "\n".join(f"- {x}" for x in r.xrefs_out) or "(none)"
    p = task_template
    p = p.replace("${module}", r.module)
    p = p.replace("${function_name}", r.name)
    p = p.replace("${bank}", bank.upper())
    p = p.replace("${offset}", offset.upper())
    p = p.replace("${asm_body}", r.asm_body)
    p = p.replace("${xrefs_out}", xrefs_str)
    p = p.replace("${xrefs_in}", "(omitted in batch mode)")
    return f"mode: {mode}\n\n{p}"


# ---------------------------------------------------------------------------
# Delegate path — emit the wrapper directly (zero token)
# ---------------------------------------------------------------------------

def emit_delegate_wrapper(r: RoutineInfo) -> str:
    # Module → SNES bank (from rom/ff4-jp1.map segment list)
    MODULE_BANK = {
        "battle":   "03",   # battle_code starts at 0x038000
        "btlgfx":   "02",   # btlgfx_code starts at 0x028000
        "menu":     "01",   # menu_code starts at 0x018000
        "field":    "00",   # field_code starts at 0x008000
        "sound":    "04",   # sound_code starts at 0x048000
        "cutscene": "13",   # cutscene_code starts at 0x13D610
    }
    bank = MODULE_BANK.get(r.module, "03")
    # The address_hint provided by ca65-bridge is the 16-bit offset within
    # the bank — we prepend the module's bank to build the full 24-bit address.
    offset = r.address.upper().lstrip("$").removeprefix("0X")
    addr24 = f"0x{bank}{offset}u"
    reasons_doc = "ADR-003 delegate reasons: " + "; ".join(r.reasons or ["heuristic"])
    return f"""\
// {r.module}::{r.name} — delegated wrapper
// {reasons_doc}
static void {r.name}_emu(Snes *snes) {{
    Cpu *c = snes->cpu;
    c->dp = 0;
    c->db = 0x7E;
    c->mf = true;
    c->xf = false;
    c->a = 0; c->x = 0; c->y = 0;
    c->z = true; c->n = false;
    run_emulated_func(snes, {addr24});
}}

// DELEGATED_FUNCTION: {r.module}::{r.name} (${bank.upper()}:{offset.upper()})
"""


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--module", required=True, help="battle, menu, field, btlgfx, sound, cutscene")
    ap.add_argument("--max-functions", type=int, default=5)
    ap.add_argument("--budget-usd", type=float, default=DEFAULT_BUDGET,
                    help="Hard budget cap. 0 = no cap (useful for claude CLI / Ollama).")
    ap.add_argument("--llm", choices=["claude-cli", "anthropic-sdk", "openai-compat"],
                    default="claude-cli", help="LLM backend (default: claude-cli)")
    ap.add_argument("--model", default=None,
                    help="Model name (per-provider default if unset)")
    ap.add_argument("--max-output-tokens", type=int, default=2000)
    ap.add_argument("--api-base", default=None,
                    help="Base URL for OpenAI-compat (e.g. http://localhost:11434/v1)")
    ap.add_argument("--api-key", default=None,
                    help="API key for OpenAI-compat (optional for Ollama)")
    ap.add_argument("--claude-bin", default="claude",
                    help="Path to the `claude` CLI binary (default: claude)")
    ap.add_argument("--dry-run", action="store_true",
                    help="Estimate tokens, do not call the API")
    ap.add_argument("--only-translate", action="store_true",
                    help="Skip routines classified as 'delegate'")
    ap.add_argument("--only-delegate", action="store_true",
                    help="Skip routines classified as 'translate'")
    args = ap.parse_args(argv)

    # Per-provider default model
    model = args.model or DEFAULT_MODELS[args.llm]

    # LLM provider
    provider = create_provider(
        args.llm,
        bin_path=args.claude_bin,
        api_base=args.api_base,
        api_key=args.api_key or os.environ.get("OPENAI_API_KEY"),
    )

    prompts = load_prompts()

    sys.stderr.write(f"[batch] enumerating module={args.module}\n")
    routines = enumerate_module(args.module)
    sys.stderr.write(f"[batch] {len(routines)} routines found\n")

    PORT_DIR.mkdir(exist_ok=True)
    out_module_dir = PORT_DIR / args.module
    out_module_dir.mkdir(exist_ok=True)

    total_cost = 0.0
    n_processed = 0
    n_translate_done = 0
    n_delegate_done = 0
    log_fp = LOG_FILE.open("a")

    for r in routines:
        if args.only_translate and r.decision != "translate":
            continue
        if args.only_delegate and r.decision != "delegate":
            continue
        if n_processed >= args.max_functions:
            sys.stderr.write(f"[batch] reached --max-functions={args.max_functions}, stopping\n")
            break

        r = hydrate(r)

        record = {
            "name": r.name,
            "module": r.module,
            "address": r.address,
            "decision": r.decision,
            "instr_count": r.instr_count,
            "call_count": r.call_count,
            "reasons": r.reasons,
        }

        if r.decision == "delegate":
            wrapper = emit_delegate_wrapper(r)
            (out_module_dir / f"{r.name}.c").write_text(wrapper)
            record["status"] = "delegate_emitted"
            record["cost_usd"] = 0.0
            n_delegate_done += 1
        else:
            user_prompt = build_user_prompt(r, "translate", prompts["task_template"])
            code, stats = provider.translate(
                system=prompts["system"],
                examples=prompts["examples"],
                user_prompt=user_prompt,
                model=model,
                max_output_tokens=args.max_output_tokens,
                dry_run=args.dry_run,
            )
            total_cost += stats.cost_usd
            record["status"] = "dry_run" if args.dry_run else ("translated" if code else "no_code_extracted")
            record["provider"] = stats.provider
            record["model"] = stats.model
            record["tokens_in"] = stats.tokens_in
            record["tokens_out"] = stats.tokens_out
            record["tokens_cache_read"] = stats.tokens_cache_read
            record["cost_usd"] = round(stats.cost_usd, 4)
            if stats.error:
                record["error"] = stats.error
            if code and not args.dry_run:
                (out_module_dir / f"{r.name}.c").write_text(code)
                n_translate_done += 1
            # Budget cap only applies when > 0 (claude CLI/Ollama have no cap).
            if args.budget_usd > 0 and total_cost > args.budget_usd:
                sys.stderr.write(f"[batch] BUDGET EXCEEDED ${total_cost:.4f} > ${args.budget_usd}, stopping\n")
                log_fp.write(json.dumps(record) + "\n")
                break

        log_fp.write(json.dumps(record) + "\n")
        print(json.dumps(record))
        n_processed += 1

    log_fp.close()

    budget_str = f"${args.budget_usd}" if args.budget_usd > 0 else "no-cap"
    sys.stderr.write(f"\n[batch] === summary ===\n")
    sys.stderr.write(f"[batch] provider:   {args.llm} (model: {model})\n")
    sys.stderr.write(f"[batch] processed:  {n_processed}\n")
    sys.stderr.write(f"[batch] translated: {n_translate_done}\n")
    sys.stderr.write(f"[batch] delegated:  {n_delegate_done}\n")
    sys.stderr.write(f"[batch] total cost: ${total_cost:.4f} (budget {budget_str})\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
