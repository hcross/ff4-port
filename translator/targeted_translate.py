#!/usr/bin/env python3
"""Targeted translator — only the named routines, not whole module.

Built on top of batch_translate.py. Reads names from --names-file (one
per line), each prefixed with module:, e.g.:
    menu:UpdateCtrl
    menu:ReadCtrl
    field:Mult16
"""
from __future__ import annotations
import argparse, json, os, re, subprocess, sys
from pathlib import Path

THIS = Path(__file__).resolve().parent
sys.path.insert(0, str(THIS))
import batch_translate as bt


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--names-file", required=True, type=Path)
    ap.add_argument("--llm", default="openai-compat")
    ap.add_argument("--api-base", default="https://ollama.com/v1")
    ap.add_argument("--api-key", default=None,
                    help="API key. Default: read from ~/.ollama/ff4-port.api.key")
    ap.add_argument("--model", default="gemma4:31b")
    ap.add_argument("--out-dir", type=Path, default=THIS.parent / "port")
    ap.add_argument("--log", type=Path, default=THIS / "runs/targeted_log.jsonl")
    ap.add_argument("--max-output-tokens", type=int, default=4096)
    args = ap.parse_args()

    targets = []
    for line in args.names_file.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if ":" not in line:
            sys.stderr.write(f"skip malformed line: {line}\n")
            continue
        mod, name = line.split(":", 1)
        targets.append((mod.strip(), name.strip()))
    sys.stderr.write(f"[targeted] {len(targets)} routines to translate\n")

    api_key = args.api_key
    if api_key is None:
        api_key = Path.home().joinpath(".ollama/ff4-port.api.key").read_text().strip()
    provider = bt.create_provider(
        args.llm, bin_path=None,
        api_base=args.api_base, api_key=api_key,
    )
    prompts = bt.load_prompts()
    args.log.parent.mkdir(exist_ok=True, parents=True)
    log_fp = args.log.open("a")

    for i, (mod, name) in enumerate(targets, 1):
        sys.stderr.write(f"[{i}/{len(targets)}] {mod}:{name} ... ")
        sys.stderr.flush()
        r = bt.RoutineInfo(name=name, module=mod, address="",
                           instr_count=0, call_count=0,
                           decision="translate", reasons=["targeted"])
        try:
            r = bt.hydrate(r)
        except Exception as e:
            sys.stderr.write(f"hydrate ERR: {e}\n")
            log_fp.write(json.dumps({"name": name, "module": mod,
                                     "status": "hydrate_error",
                                     "error": str(e)}) + "\n")
            log_fp.flush()
            continue

        if r.instr_count == 0:
            sys.stderr.write("no asm body, skip\n")
            log_fp.write(json.dumps({"name": name, "module": mod,
                                     "status": "no_asm"}) + "\n")
            log_fp.flush()
            continue

        user_prompt = bt.build_user_prompt(r, "translate", prompts["task_template"])
        try:
            code, stats = provider.translate(
                system=prompts["system"], examples=prompts["examples"],
                user_prompt=user_prompt, model=args.model,
                max_output_tokens=args.max_output_tokens, dry_run=False,
            )
        except Exception as e:
            sys.stderr.write(f"LLM ERR: {e}\n")
            log_fp.write(json.dumps({"name": name, "module": mod,
                                     "status": "llm_error",
                                     "error": str(e)}) + "\n")
            log_fp.flush()
            continue

        rec = {"name": name, "module": mod, "address": r.address,
               "instr_count": r.instr_count, "tokens_in": stats.tokens_in,
               "tokens_out": stats.tokens_out}
        if code:
            out_dir = args.out_dir / mod
            out_dir.mkdir(exist_ok=True, parents=True)
            (out_dir / f"{name}.c").write_text(bt.post_process_c(code))
            rec["status"] = "translated"
            rec["file"] = str(out_dir / f"{name}.c")
            sys.stderr.write(f"OK code_len={len(code)} in/out={stats.tokens_in}/{stats.tokens_out}\n")
        else:
            rec["status"] = "no_code"
            if stats.error:
                rec["error"] = stats.error
            sys.stderr.write(f"NO CODE err={stats.error}\n")
        log_fp.write(json.dumps(rec) + "\n")
        log_fp.flush()

    log_fp.close()
    sys.stderr.write("[targeted] done\n")


if __name__ == "__main__":
    sys.exit(main())
