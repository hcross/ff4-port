#!/usr/bin/env python3
"""P1 Calibration — measure each model's PASS rate per routine.

Cross-product: N routines × 5 tiers. Per cell: translate, validate,
classify failure. Output: calibration_matrix.jsonl + summary.

Failure classes (mutually exclusive, checked in order):
  NO_CODE         : LLM returned nothing usable
  WRONG_SIGNATURE : no `void X_c(Snes *snes)` found
  TRUNCATED       : code ends mid-statement (heuristic)
  COMPILE_ERROR   : gcc rejected the C
  HALLUCINATED    : linker said "undefined reference to <symbol>" outside our helper set
  RAM_DIVERGE     : spike ran but post-run RAM differs from oracle
  ORACLE_BLIND    : routine has CUSTOM_SPIKE flag (no oracle available)
  PASS            : all green
"""
from __future__ import annotations
import argparse, json, os, re, subprocess, sys, time
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass, asdict

THIS = Path(__file__).resolve().parent
sys.path.insert(0, str(THIS))
import batch_translate as bt

ROOT = THIS.parent
GENERATE_SPIKE = THIS / "generate_spike.py"

TIERS = [
    {"name": "T1", "model": "gemma4:31b"},
    {"name": "T2", "model": "qwen3-coder:480b"},
    {"name": "T3", "model": "glm-5.1"},
    {"name": "T4", "model": "kimi-k2.6"},
    {"name": "T5", "model": "deepseek-v4-pro"},
]

API_BASE = "https://ollama.com/v1"


@dataclass
class Cell:
    tier: str
    model: str
    module: str
    name: str
    status: str = ""
    failure_class: str = ""
    tokens_in: int = 0
    tokens_out: int = 0
    wall_sec: float = 0.0
    spike_exit: int = -1
    spike_tail: str = ""
    error: str = ""
    code_path: str = ""


_SIG_RE = re.compile(r'^\s*(?:static\s+)?(?:inline\s+)?void\s+\w+_c\s*\(\s*Snes\s*\*\s*\w+\s*\)\s*\{', re.MULTILINE)
_TRUNCATED_RE = re.compile(r'[^{};\s\)/]\s*\Z')  # ends mid-token


def classify_failure(code: str, spike_stdout: str, spike_exit: int) -> tuple[str, str]:
    """Return (failure_class, status)."""
    if not code or len(code.strip()) < 40:
        return "NO_CODE", "fail"
    if not _SIG_RE.search(code):
        return "WRONG_SIGNATURE", "fail"
    if _TRUNCATED_RE.search(code.rstrip()) and code.rstrip()[-1] not in "}":
        # code ends mid-statement — likely truncated
        if not code.rstrip().endswith("}"):
            return "TRUNCATED", "fail"
    s = spike_stdout
    if spike_exit != 0:
        # Look for specific patterns
        if "undefined reference" in s:
            m = re.search(r"undefined reference to `(\w+)'", s)
            return ("HALLUCINATED", "fail") if m and not m.group(1).endswith("_emu") else ("COMPILE_ERROR", "fail")
        if re.search(r"error:|Error 1|warning: implicit", s, re.IGNORECASE):
            return "COMPILE_ERROR", "fail"
        return "COMPILE_ERROR", "fail"
    # spike_exit == 0
    if "CUSTOM_SPIKE" in s or "custom_spike" in s:
        return "ORACLE_BLIND", "skip"
    m = re.search(r"fails:\s*(\d+)", s)
    if m and int(m.group(1)) > 0:
        return "RAM_DIVERGE", "fail"
    return "", "pass"


def parse_targets(path: Path) -> list[tuple[str, str]]:
    out = []
    for line in path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if ":" not in line:
            continue
        mod, name = line.split(":", 1)
        out.append((mod.strip(), name.strip()))
    return out


def run_cell(tier: dict, module: str, name: str, provider,
             prompts: dict, out_root: Path,
             max_output_tokens: int) -> Cell:
    cell = Cell(tier=tier["name"], model=tier["model"], module=module, name=name)
    t0 = time.time()
    r = bt.RoutineInfo(name=name, module=module, address="",
                       instr_count=0, call_count=0,
                       decision="translate", reasons=["calib"])
    try:
        r = bt.hydrate(r)
    except Exception as e:
        cell.status = "fail"
        cell.failure_class = "HYDRATE_ERROR"
        cell.error = str(e)[:200]
        cell.wall_sec = round(time.time() - t0, 2)
        return cell

    if r.instr_count == 0:
        cell.status = "skip"
        cell.failure_class = "NO_ASM"
        cell.wall_sec = round(time.time() - t0, 2)
        return cell

    user_prompt = bt.build_user_prompt(r, "translate", prompts["task_template"])
    try:
        code, stats = provider.translate(
            system=prompts["system"], examples=prompts["examples"],
            user_prompt=user_prompt, model=tier["model"],
            max_output_tokens=max_output_tokens, dry_run=False,
        )
    except Exception as e:
        cell.status = "fail"
        cell.failure_class = "LLM_ERROR"
        cell.error = str(e)[:200]
        cell.wall_sec = round(time.time() - t0, 2)
        return cell

    cell.tokens_in = stats.tokens_in
    cell.tokens_out = stats.tokens_out

    if not code:
        cell.status = "fail"
        cell.failure_class = "NO_CODE"
        if stats.error:
            cell.error = stats.error[:200]
        cell.wall_sec = round(time.time() - t0, 2)
        return cell

    # write file
    out_dir = out_root / tier["name"] / module
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / f"{name}.c"
    out_path.write_text(bt.post_process_c(code))
    cell.code_path = str(out_path)

    # validate via spike
    gen = subprocess.run(
        [sys.executable, str(GENERATE_SPIKE),
         str(out_path), "--build", "--run", "100"],
        capture_output=True, text=True, cwd=str(ROOT),
        timeout=180,
    )
    full = (gen.stdout + gen.stderr)
    cell.spike_exit = gen.returncode
    cell.spike_tail = " | ".join(full.strip().splitlines()[-3:])[:400]
    fclass, status = classify_failure(out_path.read_text(), full, gen.returncode)
    cell.failure_class = fclass
    cell.status = status
    cell.wall_sec = round(time.time() - t0, 2)
    return cell


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--targets", required=True, type=Path)
    ap.add_argument("--out-root", type=Path,
                    default=ROOT / "port/_calib")
    ap.add_argument("--log", type=Path,
                    default=THIS / "runs/calibration_matrix.jsonl")
    ap.add_argument("--workers", type=int, default=5)
    ap.add_argument("--max-output-tokens", type=int, default=8192)
    ap.add_argument("--tiers", default="T1,T2,T3,T4,T5",
                    help="Comma list of tier names to run")
    args = ap.parse_args()

    selected_tiers = [t for t in TIERS if t["name"] in args.tiers.split(",")]
    targets = parse_targets(args.targets)
    sys.stderr.write(f"[calib] {len(targets)} routines × {len(selected_tiers)} tiers = {len(targets)*len(selected_tiers)} cells\n")

    api_key = Path.home().joinpath(".ollama/ff4-port.api.key").read_text().strip()
    provider = bt.create_provider("openai-compat", bin_path=None,
                                   api_base=API_BASE, api_key=api_key)
    prompts = bt.load_prompts()

    args.log.parent.mkdir(exist_ok=True, parents=True)
    log_fp = args.log.open("a")

    jobs = [(t, m, n) for t in selected_tiers for (m, n) in targets]
    done = 0
    total = len(jobs)
    t_start = time.time()

    with ThreadPoolExecutor(max_workers=args.workers) as pool:
        futures = {pool.submit(run_cell, t, m, n, provider, prompts,
                               args.out_root, args.max_output_tokens): (t, m, n)
                   for (t, m, n) in jobs}
        for fut in as_completed(futures):
            cell = fut.result()
            done += 1
            log_fp.write(json.dumps(asdict(cell)) + "\n")
            log_fp.flush()
            elapsed = time.time() - t_start
            sys.stderr.write(f"[{done}/{total}] {cell.tier} {cell.module}:{cell.name} -> "
                             f"{cell.status}/{cell.failure_class} ({cell.wall_sec}s) "
                             f"[wall={int(elapsed)}s]\n")

    log_fp.close()
    sys.stderr.write(f"[calib] done in {int(time.time()-t_start)}s, log={args.log}\n")


if __name__ == "__main__":
    sys.exit(main())
