#!/usr/bin/env python3
"""Hardcore translator — deepseek-v4-pro + reverser_hardcore.md.

For routines that the standard P3 v2 pipeline (gemma4:31b + multi-pass
critic) cannot crack — typically those that surface compile_error or
ram_diverge on every temperature. Uses deepseek-v4-pro on Ollama Cloud
($0 marginal) with a much richer system prompt that includes:

  - Extended Snes/Cpu/Ppu/Apu API reference (every field that can be
    referenced verbatim)
  - Common-hallucinations table (wrong vs correct names)
  - Three additional few-shot examples for hard patterns (data table,
    SPC handshake, PPU MMIO)
  - A 4-step self-check the model must run before emitting
  - Explicit "when in doubt, delegate" gating

Each routine is translated then validated via generate_spike. Output
goes to port/_hardcore/<mod>/ to keep the artefacts separate from the
gemma4 pipeline's port/_runs/ tree.

Usage:
    python translator/hardcore_translate.py \\
        --names battle:Special_06 field:WaitSpecial menu:UpdateCtrl \\
        --out-dir port/_hardcore

    # Or from a file with mod:name per line:
    python translator/hardcore_translate.py --names-file /tmp/hard.txt
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import tempfile
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


def translate_one(mod: str, name: str, system_md: str, examples_md: str,
                   task_template: str, out_dir: Path, provider,
                   model: str, max_output_tokens: int,
                   timeout: int = 240) -> dict:
    """Mirror batch_translate.translate_one but with the hardcore prompts
    inlined."""
    out_dir = out_dir / mod
    out_dir.mkdir(parents=True, exist_ok=True)
    target_c = out_dir / f"{name}.c"

    routine = bt.RoutineInfo(
        name=name, module=mod, address="", instr_count=0, call_count=0,
        decision="translate", reasons=["hardcore"],
        xrefs_out=[], asm_body="",
    )
    try:
        routine = bt.hydrate(routine)
    except Exception as e:
        return {"mod": mod, "name": name, "status": "hydrate_error",
                "error": str(e)[:300]}
    if not routine.asm_body or len(routine.asm_body) < 30:
        return {"mod": mod, "name": name, "status": "asm_empty",
                "address": routine.address,
                "note": "ca65-bridge returned no asm — routine is probably data"}
    user_prompt = bt.build_user_prompt(routine, "translate", task_template)
    code, stats = provider.translate(
        system=system_md, examples=examples_md,
        user_prompt=user_prompt, model=model,
        max_output_tokens=max_output_tokens,
        dry_run=False,
    )
    if not code:
        return {"mod": mod, "name": name, "status": "no_code",
                "stats": stats.__dict__ if stats else {}}
    # batch_translate strips `static` so the symbol can be linked.
    code = re.sub(r"^\s*static\s+(?=void\s+\w+_c\s*\()", "", code,
                   count=1, flags=re.MULTILINE)
    target_c.write_text(code)
    return {"mod": mod, "name": name, "status": "translated",
            "c_path": str(target_c),
            "stats": stats.__dict__ if stats else {}}


def validate_one(c_path: Path, tag: str, timeout: int = 90) -> dict:
    try:
        proc = subprocess.run(
            [sys.executable, str(GENERATE_SPIKE),
             str(c_path), "--build", "--run", "100",
             "--spike-suffix", tag],
            capture_output=True, text=True, cwd=str(ROOT), timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return {"status": "spike_timeout"}
    out = proc.stdout + "\n--- stderr ---\n" + proc.stderr
    m = re.search(r"=== summary === trials: (\d+), fails: (\d+)", out)
    if m and int(m.group(2)) == 0:
        return {"status": "pass", "trials": int(m.group(1))}
    if m:
        return {"status": "ram_diverge", "trials": int(m.group(1)),
                "fails": int(m.group(2))}
    if "delegate wrapper" in out:
        return {"status": "delegate_pass",
                "note": "deepseek chose delegate stub (correct under H4)"}
    if "skipping auto-gen" in out or "CUSTOM_SPIKE" in out:
        return {"status": "custom_spike", "tail": out[-600:]}
    if "implicit-function-declaration" in out or "undeclared" in out:
        return {"status": "compile_error", "tail": out[-1200:]}
    if "make: ***" in out or "Error 1" in out:
        return {"status": "compile_error", "tail": out[-1200:]}
    return {"status": "unknown", "tail": out[-600:]}


def main():
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--names", nargs="+", help="mod:name entries to translate")
    g.add_argument("--names-file", type=Path,
                   help="one mod:name per line (# comments and blanks ok)")
    ap.add_argument("--model", default=DEFAULT_MODEL)
    ap.add_argument("--system-md", type=Path, default=DEFAULT_HARDCORE_PROMPT)
    ap.add_argument("--task-md", type=Path, default=DEFAULT_TASK_TEMPLATE)
    ap.add_argument("--examples-md", type=Path, default=DEFAULT_EXAMPLES)
    ap.add_argument("--out-dir", type=Path, default=ROOT / "port" / "_hardcore")
    ap.add_argument("--api-base", default=DEFAULT_API_BASE)
    ap.add_argument("--api-key", default=None)
    ap.add_argument("--max-output-tokens", type=int, default=12288)
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
    sys.stderr.write(f"[hardcore] {len(targets)} routines to translate "
                     f"via {args.model}\n")

    provider = bt.create_provider("openai-compat", bin_path=None,
                                   api_base=args.api_base, api_key=api_key)
    system_md = args.system_md.read_text()
    examples_md = args.examples_md.read_text() if args.examples_md.exists() else ""
    task_template = args.task_md.read_text()
    sys.stderr.write(f"[hardcore] system_md {len(system_md)} bytes, "
                     f"examples {len(examples_md)} bytes\n")

    args.log.parent.mkdir(parents=True, exist_ok=True)
    log_fp = args.log.open("a")
    summary = {"pass": 0, "ram_diverge": 0, "compile_error": 0,
               "custom_spike": 0, "no_code": 0, "unknown": 0,
               "spike_timeout": 0}

    for i, (mod, name) in enumerate(targets, 1):
        sys.stderr.write(f"\n[{i}/{len(targets)}] {mod}:{name} ...\n")
        sys.stderr.flush()
        t = translate_one(mod, name, system_md, examples_md, task_template,
                          args.out_dir, provider, args.model,
                          args.max_output_tokens)
        if t["status"] != "translated":
            sys.stderr.write(f"  -> {t['status']}\n")
            summary[t["status"]] = summary.get(t["status"], 0) + 1
            log_fp.write(json.dumps({"i": i, "mod": mod, "name": name,
                                       "phase": "translate", **t}) + "\n")
            continue
        v = validate_one(Path(t["c_path"]),
                          tag=f"hardcore_{name.lower()}", timeout=90)
        rec = {"i": i, "mod": mod, "name": name, "phase": "validate",
               "c_path": t["c_path"], "tokens_out": t["stats"].get("tokens_out", 0),
               **v}
        sys.stderr.write(f"  -> {v['status']}\n")
        summary[v["status"]] = summary.get(v["status"], 0) + 1
        log_fp.write(json.dumps(rec) + "\n")
        log_fp.flush()

    log_fp.close()
    sys.stderr.write(f"\n[hardcore] summary: {summary}\n")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
