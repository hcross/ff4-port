#!/usr/bin/env python3
"""Validate every C translation under port/_runs/<model>/<module>/ via
generate_spike.py --build --run 100 and emit a per-run statistics block.

Spike outputs are tagged with the model name (--spike-suffix) so parallel
or sequential runs don't overwrite each other's spike binaries.

Usage:
    python translator/validate_run.py \\
        --run-dir port/_runs/qwen3-coder_480b \\
        --tag qwen3
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
GENERATE_SPIKE = HERE / "generate_spike.py"


def validate_one(c_path: Path, tag: str, per_call_timeout: int = 30) -> dict:
    """Run generate_spike --build --run 100 and parse the verdict.

    Wrapped in a hard timeout so that an LLM-produced C body containing an
    infinite loop (which makes the spike binary spin forever) cannot stall
    the batch. On timeout we return status="timeout" and move on.
    """
    try:
        proc = subprocess.run(
            [sys.executable, str(GENERATE_SPIKE),
             str(c_path), "--build", "--run", "100",
             "--spike-suffix", tag],
            capture_output=True, text=True, cwd=str(ROOT),
            timeout=per_call_timeout,
        )
    except subprocess.TimeoutExpired:
        return {
            "file": str(c_path),
            "name": c_path.stem,
            "status": "timeout",
            "exit": -1,
        }
    stdout = proc.stdout
    stderr = proc.stderr
    try:
        rel = str(c_path.resolve().relative_to(ROOT))
    except ValueError:
        rel = str(c_path)
    record = {
        "file": rel,
        "name": c_path.stem,
        "exit": proc.returncode,
    }
    # Detect skip reasons emitted on stderr.
    if "skipping auto-gen" in stderr:
        if "CUSTOM_SPIKE" in stderr:
            record["status"] = "custom_spike"
        elif "indexed store" in stderr:
            record["status"] = "indexed_store_skip"
        else:
            record["status"] = "skipped"
        return record
    if "delegate wrapper" in stderr:
        record["status"] = "delegate"
        return record
    if "could not parse a CONTRACT" in stderr:
        record["status"] = "no_contract"
        return record
    # Compile / run outcomes.
    m = re.search(r"fails:\s*(\d+)", stdout)
    if proc.returncode == 0 and m is not None:
        record["fails"] = int(m.group(1))
        record["status"] = "pass" if record["fails"] == 0 else "fail"
        return record
    if proc.returncode != 0 and "Error 1" in stderr:
        record["status"] = "compile_error"
        # First reported compile error line
        m_err = re.search(r"error: ([^\n]+)", stderr)
        if m_err:
            record["error_message"] = m_err.group(1)[:200]
        return record
    record["status"] = "unknown"
    record["stderr_tail"] = stderr.strip().splitlines()[-2:]
    return record


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--run-dir", type=Path, required=True,
                    help="Root of a translation run (e.g. port/_runs/<model>)")
    ap.add_argument("--tag", required=True,
                    help="Spike suffix and label for this run (e.g. qwen3, gemma4)")
    ap.add_argument("--module", default="battle",
                    help="Module subdirectory to walk (default: battle)")
    ap.add_argument("--per-call-timeout", type=int, default=30,
                    help="Per-routine hard timeout in seconds. If the spike "
                         "binary spins (LLM-produced infinite loop), we kill "
                         "it and record status=timeout.")
    args = ap.parse_args()

    module_dir = args.run_dir / args.module
    if not module_dir.is_dir():
        sys.stderr.write(f"no such dir: {module_dir}\n")
        return 1

    c_files = sorted(module_dir.glob("*.c"))
    sys.stderr.write(f"[validate_run] {args.tag}: {len(c_files)} C files to validate\n")

    counts = Counter()
    rows = []
    for c in c_files:
        rec = validate_one(c, args.tag, per_call_timeout=args.per_call_timeout)
        rows.append(rec)
        counts[rec["status"]] += 1
        print(json.dumps(rec), flush=True)

    sys.stderr.write(f"\n[validate_run] {args.tag} summary:\n")
    for status, n in counts.most_common():
        sys.stderr.write(f"  {status:18s} {n:4d}\n")
    sys.stderr.write(f"  TOTAL              {len(rows):4d}\n")
    pass_n = counts.get("pass", 0)
    if rows:
        sys.stderr.write(f"  pass rate          {100 * pass_n / len(rows):.1f}%\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
