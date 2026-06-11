#!/usr/bin/env python3
"""Bench multiple Ollama Cloud models on the same translation task.

Output: JSONL per-model record to stdout + per-model port file under
port/_bench/<model>/<func>.c. Optionally also runs generate_spike.py on
each candidate (--validate) and reports PASS/FAIL.

The Ollama Cloud API key is read from the OPENAI_API_KEY environment
variable. Pass it via the shell so the value never enters this process's
argv or this script's traceback.

Usage:
    OPENAI_API_KEY="$(cat ~/.ollama/<key-file>)" \\
        python translator/bench_models.py \\
            --target battle:AICond_02 \\
            --models qwen3-coder:480b,deepseek-v3.2,glm-4.7 \\
            --validate
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
PORT_BENCH = ROOT / "port" / "_bench"

sys.path.insert(0, str(HERE))
from batch_translate import enumerate_module, hydrate, build_user_prompt, load_prompts
from llm_providers import create_provider


def run_one(model: str, target_module: str, target_name: str,
            max_output_tokens: int, validate: bool) -> dict:
    routines = enumerate_module(target_module)
    target = next(r for r in routines if r.name == target_name)
    target = hydrate(target)
    prompts = load_prompts()
    user_prompt = build_user_prompt(target, "translate", prompts["task_template"])

    api_key = os.environ.get("OPENAI_API_KEY")
    if not api_key:
        return {"model": model, "error": "OPENAI_API_KEY not set"}

    provider = create_provider(
        "openai-compat",
        api_base="https://ollama.com/v1",
        api_key=api_key,
    )

    start = time.time()
    try:
        code, stats = provider.translate(
            prompts["system"], prompts["examples"], user_prompt,
            model, max_output_tokens, dry_run=False,
        )
    except Exception as e:
        return {"model": model, "error": f"exception: {e!r}", "elapsed_s": round(time.time() - start, 1)}
    elapsed = time.time() - start

    record = {
        "model": model,
        "elapsed_s": round(elapsed, 1),
        "tokens_in": stats.tokens_in,
        "tokens_out": stats.tokens_out,
        "code_extracted": code is not None,
        "error": stats.error,
    }

    if code is None:
        return record

    # Save to port/_bench/<sanitised>/<func>.c
    safe_model = model.replace("/", "_").replace(":", "_")
    out_dir = PORT_BENCH / safe_model / target_module
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / f"{target_name}.c"
    out_path.write_text(code)
    record["file"] = str(out_path.relative_to(ROOT))

    # Quick QA over the produced source.
    accented = sum(
        1 for line in code.splitlines()
        if any(ch in line for ch in "éèêëàâäôöûüçîïùÉÈÊËÀÂÄÔÖÛÜÇÎÏÙ")
    )
    record["accented_lines"] = accented
    record["has_contract_block"] = "// CONTRACT:" in code
    record["has_custom_spike"] = "CUSTOM_SPIKE: yes" in code
    record["has_reversed_function"] = "REVERSED_FUNCTION:" in code

    if validate and record["has_contract_block"]:
        gen = subprocess.run(
            [sys.executable, str(HERE / "generate_spike.py"),
             str(out_path), "--build", "--run", "100"],
            capture_output=True, text=True,
            cwd=str(ROOT),
        )
        record["spike_exit"] = gen.returncode
        # Parse fail count from tail
        out_tail = (gen.stdout + gen.stderr).strip().splitlines()[-5:]
        record["spike_tail"] = " | ".join(out_tail)

    return record


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--target", required=True, help="module:function (e.g. battle:AICond_02)")
    ap.add_argument("--models", required=True,
                    help="comma-separated Ollama model ids")
    ap.add_argument("--max-output-tokens", type=int, default=4000)
    ap.add_argument("--validate", action="store_true",
                    help="run generate_spike.py + 100-trial fuzz on each result")
    args = ap.parse_args()

    if ":" not in args.target:
        sys.stderr.write("--target must be module:function\n")
        return 1
    module, name = args.target.split(":", 1)

    for m in args.models.split(","):
        m = m.strip()
        if not m:
            continue
        rec = run_one(m, module, name, args.max_output_tokens, args.validate)
        print(json.dumps(rec))
    return 0


if __name__ == "__main__":
    sys.exit(main())
