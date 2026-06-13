#!/usr/bin/env python3
"""P3 prompt-mutation loop (ADR-004).

For each routine that FAILS the baseline translation pipeline, ask
gpt-oss-120b to propose a new system prompt that would have helped that
routine pass. Re-test the routine with the candidate prompt. If it passes,
run the regression suite to make sure no previously-passing routine
regressed. If the regression suite is intact (STRICT: 0 regression
tolerated), adopt the candidate as the new prompt vK+1.

Stop criteria (any one triggers exit):
  - pass_rate_target reached on the regression suite
  - max_iterations exhausted
  - max_consecutive_rejects in a row

Outputs:
  - prompts/history/v1/, v2/, ... — each adopted version
  - prompts/history/rejected/ — proposed but rejected candidates
  - translator/runs/mutation_log.jsonl — per-iteration audit trail

Usage:
    python translator/prompt_mutation_loop.py \\
        --start-version v0 \\
        --max-iterations 5 \\
        --pass-rate-target 0.50
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path


THIS = Path(__file__).resolve().parent
ROOT = THIS.parent
PROMPTS_DIR = ROOT / "prompts"
HISTORY_DIR = PROMPTS_DIR / "history"
CRITIC = THIS / "gpt_oss_critic.py"
REGRESSION = THIS / "regression_suite.py"
TARGETED = THIS / "targeted_translate.py"
GENERATE_SPIKE = THIS / "generate_spike.py"
MUTATION_LOG = THIS / "runs" / "mutation_log.jsonl"


def load_fail_routines(jsonl_path: Path, tier_filter: str = "gemma4:31b") -> list[dict]:
    """Read a calibration matrix and return routines that FAIL on the
    specified tier. Each item: {name, mod, status, error_message}."""
    fails = []
    for line in jsonl_path.read_text().splitlines():
        if not line.strip():
            continue
        try:
            rec = json.loads(line)
        except json.JSONDecodeError:
            continue
        # calibration_matrix_v2.jsonl format: status=pass/fail, tier_model or model field
        model = rec.get("tier_model") or rec.get("model") or ""
        if tier_filter and tier_filter not in model:
            continue
        if rec.get("status") in ("pass", "PASS"):
            continue
        fails.append({
            "name": rec.get("name", "?"),
            "mod": rec.get("mod") or rec.get("module") or guess_mod(rec.get("name", "")),
            "status": rec.get("status"),
            "error_class": rec.get("error_class") or rec.get("class") or "UNKNOWN",
            "error_message": rec.get("error_message") or rec.get("stderr") or "",
        })
    return fails


def guess_mod(name: str) -> str:
    """Cheap heuristic: look up which port/<mod>/ contains <name>.c, fallback
    to 'battle'."""
    for mod in ["battle", "field", "menu", "cutscene", "sound", "btlgfx"]:
        if (ROOT / "port" / mod / f"{name}.c").exists():
            return mod
        if (ROOT / "upstream" / mod / f"{name.lower()}.asm").exists():
            return mod
    return "battle"


def translate_then_validate(mod: str, name: str, prompts_dir: Path,
                              out_dir: Path, model: str, api_key: str,
                              translate_timeout: int = 120,
                              validate_timeout: int = 60) -> dict:
    """Run targeted_translate then generate_spike for a single routine."""
    import tempfile
    out_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", suffix=".txt", delete=False) as tf:
        tf.write(f"{mod}:{name}\n")
        names_file = Path(tf.name)
    try:
        cmd = [
            sys.executable, str(TARGETED),
            "--names-file", str(names_file),
            "--out-dir", str(out_dir),
            "--model", model,
            "--prompts-dir", str(prompts_dir),
            "--api-key", api_key,
        ]
        proc = subprocess.run(cmd, cwd=str(ROOT), capture_output=True,
                              text=True, timeout=translate_timeout)
        if proc.returncode != 0:
            return {"status": "translate_fail", "stderr_tail": proc.stderr[-1000:]}
        c_path = out_dir / mod / f"{name}.c"
        if not c_path.exists():
            return {"status": "no_output", "stderr_tail": proc.stderr[-500:]}
    except subprocess.TimeoutExpired:
        return {"status": "translate_timeout"}
    finally:
        names_file.unlink(missing_ok=True)

    try:
        proc = subprocess.run(
            [sys.executable, str(GENERATE_SPIKE),
             str(c_path), "--build", "--run", "100",
             "--spike-suffix", f"mut_{prompts_dir.name}"],
            capture_output=True, text=True, cwd=str(ROOT),
            timeout=validate_timeout,
        )
    except subprocess.TimeoutExpired:
        return {"status": "validate_timeout", "c_path": str(c_path)}
    out = proc.stdout
    if "VERDICT: PASS" in out or '"status": "pass"' in out:
        return {"status": "pass", "c_path": str(c_path)}
    if "compile error" in out.lower() or "compilation failed" in out.lower():
        return {"status": "compile_error", "c_path": str(c_path),
                "tail": out[-800:]}
    return {"status": "fail", "c_path": str(c_path), "tail": out[-800:]}


def call_critic(current_system_md: Path, asm_file: Path, routine_name: str,
                  generated_c: Path, error_class: str, error_message: str,
                  out_system_md: Path, api_key: str,
                  timeout: int = 600) -> bool:
    """Call gpt_oss_critic.py. Returns True if a new prompt was produced."""
    cmd = [
        sys.executable, str(CRITIC),
        "--system-md", str(current_system_md),
        "--asm-file", str(asm_file),
        "--routine-name", routine_name,
        "--generated-c", str(generated_c),
        "--error-class", error_class,
        "--error-message", error_message[:2000],
        "--out-system-md", str(out_system_md),
        "--api-key", api_key,
    ]
    try:
        proc = subprocess.run(cmd, cwd=str(ROOT), capture_output=True,
                              text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        sys.stderr.write("[critic] timeout\n")
        return False
    sys.stderr.write(proc.stderr)
    return out_system_md.exists() and out_system_md.stat().st_size > 200


def run_regression(prompts_dir: Path, out_dir: Path, model: str,
                    api_key: str, parallel: int = 4) -> dict:
    """Invoke regression_suite.py as a subprocess and return its JSON
    report."""
    json_out = THIS / "runs" / f"reg_{prompts_dir.name}.json"
    cmd = [
        sys.executable, str(REGRESSION),
        "--prompts-dir", str(prompts_dir),
        "--out-dir", str(out_dir),
        "--model", model,
        "--api-key", api_key,
        "--parallel", str(parallel),
        "--json-out", str(json_out),
    ]
    proc = subprocess.run(cmd, cwd=str(ROOT), capture_output=False)
    if proc.returncode != 0 or not json_out.exists():
        return {"error": "regression failed", "returncode": proc.returncode}
    return json.loads(json_out.read_text())


def find_asm_file(mod: str, routine_name: str) -> Path | None:
    """Find the upstream/<mod>/*.asm that contains the routine label."""
    mod_dir = ROOT / "upstream" / mod
    if not mod_dir.is_dir():
        return None
    for asm in mod_dir.rglob("*.asm"):
        try:
            if f"{routine_name}:" in asm.read_text():
                return asm
        except (UnicodeDecodeError, OSError):
            continue
    return None


def latest_version() -> str:
    """Return the highest-numbered vK in prompts/history/."""
    vs = sorted(p.name for p in HISTORY_DIR.iterdir()
                if p.is_dir() and p.name.startswith("v")
                and p.name[1:].isdigit())
    return vs[-1] if vs else "v0"


def next_version(current: str) -> str:
    """v3 -> v4"""
    n = int(current.lstrip("v"))
    return f"v{n + 1}"


def write_manifest(version_dir: Path, parent: str, mutation_summary: str,
                    regression_score: dict, routine_passed: str) -> None:
    """Persist the manifest.json for this adopted version."""
    manifest = {
        "version": version_dir.name,
        "parent": parent,
        "created_at": datetime.now(timezone.utc).isoformat(),
        "mutation_summary": mutation_summary,
        "routine_that_drove_mutation": routine_passed,
        "regression_score": {
            "pass_count": regression_score.get("pass_count"),
            "suite_size": regression_score.get("suite_size"),
            "pass_rate": regression_score.get("pass_rate"),
            "by_class": regression_score.get("by_class"),
        },
        "files": ["reverser_system.md", "reverser_task.md", "reverser_examples.md"],
    }
    (version_dir / "manifest.json").write_text(json.dumps(manifest, indent=2))


def adopt_candidate(candidate_system_md: Path, current_dir: Path,
                     new_version: str, mutation_summary: str,
                     regression_score: dict, routine_passed: str) -> Path:
    """Promote the candidate to prompts/history/v{K+1}/ and update the live
    prompts/ symlink-style mirror."""
    new_dir = HISTORY_DIR / new_version
    new_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy(candidate_system_md, new_dir / "reverser_system.md")
    shutil.copy(current_dir / "reverser_task.md", new_dir / "reverser_task.md")
    shutil.copy(current_dir / "reverser_examples.md",
                new_dir / "reverser_examples.md")
    write_manifest(new_dir, current_dir.name, mutation_summary,
                   regression_score, routine_passed)
    # mirror live prompts/
    shutil.copy(new_dir / "reverser_system.md",
                PROMPTS_DIR / "reverser_system.md")
    return new_dir


def log_iteration(record: dict) -> None:
    MUTATION_LOG.parent.mkdir(parents=True, exist_ok=True)
    with MUTATION_LOG.open("a") as f:
        f.write(json.dumps(record) + "\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--start-version", default=None,
                    help="prompts/history/<this>/ to start from. Defaults to latest.")
    ap.add_argument("--fail-source",
                    default="translator/runs/calibration_matrix_v2.jsonl",
                    type=Path,
                    help="JSONL with FAIL routines to attempt to fix")
    ap.add_argument("--tier-filter", default="gemma4:31b",
                    help="only consider FAIL records matching this tier_model")
    ap.add_argument("--model", default="gemma4:31b",
                    help="target LLM (the one to improve)")
    ap.add_argument("--max-iterations", type=int, default=5)
    ap.add_argument("--pass-rate-target", type=float, default=0.50)
    ap.add_argument("--max-consecutive-rejects", type=int, default=10)
    ap.add_argument("--regression-parallel", type=int, default=4)
    ap.add_argument("--api-key", default=None)
    ap.add_argument("--out-root", type=Path,
                    default=THIS.parent / "port" / "_mutation")
    args = ap.parse_args()

    api_key = args.api_key
    if api_key is None:
        kp = Path.home() / ".ollama" / "ff4-port.api.key"
        if kp.exists():
            api_key = kp.read_text().strip()
    if not api_key:
        sys.exit("no api key (--api-key or ~/.ollama/ff4-port.api.key)")

    start = args.start_version or latest_version()
    current_dir = HISTORY_DIR / start
    if not current_dir.is_dir():
        sys.exit(f"no such version dir: {current_dir}")

    fails = load_fail_routines(args.fail_source, args.tier_filter)
    sys.stderr.write(f"[loop] start={start}  fails_pool={len(fails)}  "
                     f"target={args.pass_rate_target:.0%}\n")

    rejects_in_a_row = 0
    K = int(start.lstrip("v"))
    last_regression = None
    for it in range(args.max_iterations):
        if not fails:
            sys.stderr.write("[loop] FAIL pool exhausted, stopping\n")
            break
        fail = fails.pop(0)
        mod, name = fail["mod"], fail["name"]
        sys.stderr.write(f"\n[loop] iter {it+1}/{args.max_iterations} "
                         f"v{K} -> v{K+1} on {mod}:{name}\n")

        asm_file = find_asm_file(mod, name)
        if asm_file is None:
            sys.stderr.write(f"  -> no asm file for {mod}:{name}, skipping\n")
            log_iteration({"iter": it, "fail": fail, "verdict": "no_asm"})
            continue

        # Step 1 — confirm baseline still fails
        bl_dir = args.out_root / f"baseline_v{K}_{name}"
        bl = translate_then_validate(mod, name, current_dir, bl_dir,
                                       args.model, api_key)
        if bl["status"] == "pass":
            sys.stderr.write(f"  -> already PASSES under v{K} (good news), skipping mutation\n")
            log_iteration({"iter": it, "fail": fail, "verdict": "already_pass"})
            continue
        sys.stderr.write(f"  baseline: {bl['status']}\n")

        # Step 2 — ask critic for a mutated prompt
        candidate_dir = HISTORY_DIR / f"_candidate_v{K+1}"
        candidate_dir.mkdir(parents=True, exist_ok=True)
        new_system = candidate_dir / "reverser_system.md"
        ok = call_critic(
            current_dir / "reverser_system.md",
            asm_file, name,
            Path(bl.get("c_path", "/dev/null")),
            fail["error_class"], fail["error_message"],
            new_system, api_key,
        )
        if not ok:
            sys.stderr.write("  -> critic produced no candidate\n")
            log_iteration({"iter": it, "fail": fail, "verdict": "critic_empty"})
            rejects_in_a_row += 1
            if rejects_in_a_row >= args.max_consecutive_rejects:
                break
            continue
        # carry over task + examples unchanged for now
        shutil.copy(current_dir / "reverser_task.md",
                    candidate_dir / "reverser_task.md")
        shutil.copy(current_dir / "reverser_examples.md",
                    candidate_dir / "reverser_examples.md")

        # Step 3 — re-test the FAIL routine with candidate
        cand_dir_out = args.out_root / f"candidate_v{K+1}_{name}"
        ct = translate_then_validate(mod, name, candidate_dir, cand_dir_out,
                                       args.model, api_key)
        if ct["status"] != "pass":
            sys.stderr.write(f"  -> candidate STILL fails on {name} "
                             f"({ct['status']}), rejecting\n")
            log_iteration({"iter": it, "fail": fail,
                           "verdict": "candidate_fail",
                           "candidate_status": ct["status"]})
            rejects_in_a_row += 1
            if rejects_in_a_row >= args.max_consecutive_rejects:
                break
            continue

        sys.stderr.write(f"  ✓ candidate passes on {name}\n")

        # Step 4 — regression suite
        reg_out = args.out_root / f"regression_v{K+1}"
        reg = run_regression(candidate_dir, reg_out, args.model, api_key,
                              parallel=args.regression_parallel)
        cand_pass = reg.get("pass_count", -1)
        cand_total = reg.get("suite_size", -1)
        sys.stderr.write(f"  regression v{K+1} candidate: "
                         f"{cand_pass}/{cand_total}\n")

        baseline_pass = (last_regression or {}).get("pass_count")
        if baseline_pass is None:
            # bootstrap: score the current_dir as baseline once
            sys.stderr.write("  bootstrapping baseline regression for v"
                             f"{K}\n")
            reg_baseline_out = args.out_root / f"regression_v{K}_baseline"
            last_regression = run_regression(current_dir, reg_baseline_out,
                                              args.model, api_key,
                                              parallel=args.regression_parallel)
            baseline_pass = last_regression.get("pass_count", -1)
            sys.stderr.write(f"  baseline v{K} regression: "
                             f"{baseline_pass}/{last_regression.get('suite_size', -1)}\n")

        if cand_pass < baseline_pass:
            sys.stderr.write(f"  -> STRICT regression detected "
                             f"({cand_pass} < {baseline_pass}), rejecting\n")
            log_iteration({
                "iter": it, "fail": fail, "verdict": "regression",
                "baseline_pass": baseline_pass, "cand_pass": cand_pass,
                "cand_total": cand_total,
            })
            rejects_in_a_row += 1
            if rejects_in_a_row >= args.max_consecutive_rejects:
                break
            continue

        # Step 5 — adopt
        new_version = f"v{K+1}"
        mut_summary = f"fixed {mod}:{name} ({fail['error_class']})"
        new_dir = adopt_candidate(new_system, current_dir, new_version,
                                    mut_summary, reg, f"{mod}:{name}")
        sys.stderr.write(f"  ✓✓ ADOPTED {new_version}: {mut_summary}\n")
        log_iteration({
            "iter": it, "fail": fail, "verdict": "adopted",
            "new_version": new_version,
            "baseline_pass": baseline_pass, "cand_pass": cand_pass,
            "cand_total": cand_total,
            "mutation_summary": mut_summary,
        })
        K += 1
        current_dir = new_dir
        last_regression = reg
        rejects_in_a_row = 0

        if reg.get("pass_rate", 0) >= args.pass_rate_target:
            sys.stderr.write(
                f"[loop] pass_rate_target {args.pass_rate_target:.0%} reached "
                f"({reg['pass_rate']:.0%}) — stopping\n"
            )
            break

    sys.stderr.write(f"\n[loop] done. final version: v{K}\n")
    if last_regression:
        sys.stderr.write(f"[loop] final regression: "
                         f"{last_regression.get('pass_count')}/"
                         f"{last_regression.get('suite_size')}\n")


if __name__ == "__main__":
    main()
