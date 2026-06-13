#!/usr/bin/env python3
"""Regression suite for the P3 prompt-mutation loop (ADR-004).

Given a prompts directory (e.g. prompts/history/v3/), retranslate a fixed
set of routines stratified across modules, validate each via the auto-spike
harness, and report PASS/FAIL counts. The runner uses this to decide
whether a mutated prompt regresses against the baseline.

Suite design: 19 routines (4 battle + 4 cutscene + 4 field + 4 menu + 3
sound). All chosen because they currently PASS on v0 (baseline). InitSound_ext
and ExecSound_ext are deliberately excluded — they're on the dispatch skip
list (the C body is meant to be a no-op delegate, see ff4-gnw commit
17823b7).

Usage:
    python translator/regression_suite.py \\
        --prompts-dir prompts/history/v0/ \\
        --out-dir port/_regression/v0_baseline/ \\
        --json-out /tmp/v0_score.json
"""
from __future__ import annotations

import argparse
import concurrent.futures as cf
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path


THIS = Path(__file__).resolve().parent
ROOT = THIS.parent
TARGETED = THIS / "targeted_translate.py"
GENERATE_SPIKE = THIS / "generate_spike.py"


DEFAULT_SUITE = [
    # Battle (4)
    ("battle", "AICond_00"),
    ("battle", "AICond_02"),
    ("battle", "AITarget_19"),
    ("battle", "AITarget_1a"),
    # Cutscene (4)
    ("cutscene", "GetEarthSpritePos"),
    ("cutscene", "GetOtherPlanetTile"),
    ("cutscene", "InitStars"),
    ("cutscene", "NewLine"),
    # Field (4)
    ("field", "DrawPos"),
    ("field", "HexToDec"),
    ("field", "InitSpellLists"),
    ("field", "Mult16"),
    # Menu (4)
    ("menu", "BtnAction"),
    ("menu", "BtnDefault"),
    ("menu", "ClearText"),
    ("menu", "InitCtrl"),
    # Sound (3) — InitSound_ext + ExecSound_ext excluded (dispatch skip list)
    ("sound", "ExecInterrupt"),
    ("sound", "PlayGameSfx"),
    ("sound", "PlaySystemSfx"),
]


def translate_one(mod: str, name: str, prompts_dir: Path, out_dir: Path,
                  model: str, api_key: str, timeout: int) -> dict:
    """Translate a single routine via targeted_translate. Returns the routine's
    written .c path or an error dict."""
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
        ]
        if api_key:
            cmd += ["--api-key", api_key]
        proc = subprocess.run(cmd, cwd=str(ROOT), capture_output=True,
                              text=True, timeout=timeout)
        if proc.returncode != 0:
            return {"status": "translate_fail", "stderr": proc.stderr[-2000:]}
        c_path = out_dir / mod / f"{name}.c"
        if not c_path.exists():
            return {"status": "no_output", "stderr": proc.stderr[-500:]}
        return {"status": "translated", "c_path": str(c_path)}
    except subprocess.TimeoutExpired:
        return {"status": "translate_timeout"}
    finally:
        names_file.unlink(missing_ok=True)


def validate_one(c_path: Path, tag: str, timeout: int) -> dict:
    """Run generate_spike --build --run 100 and parse the verdict (mirrors
    validate_run.py:validate_one)."""
    try:
        proc = subprocess.run(
            [sys.executable, str(GENERATE_SPIKE),
             str(c_path), "--build", "--run", "100",
             "--spike-suffix", tag],
            capture_output=True, text=True, cwd=str(ROOT), timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return {"status": "timeout"}
    out = proc.stdout
    if "VERDICT: PASS" in out or '"status": "pass"' in out:
        return {"status": "pass"}
    if "compile error" in out.lower() or "compilation failed" in out.lower():
        return {"status": "compile_error", "tail": out[-500:]}
    if proc.returncode != 0:
        return {"status": "fail", "tail": out[-500:]}
    return {"status": "unknown", "tail": out[-500:]}


def score_routine(mod: str, name: str, prompts_dir: Path, out_dir: Path,
                   model: str, api_key: str,
                   translate_timeout: int = 120,
                   validate_timeout: int = 60) -> dict:
    """End-to-end : translate then validate a single routine."""
    t = translate_one(mod, name, prompts_dir, out_dir, model, api_key,
                      translate_timeout)
    if t["status"] != "translated":
        return {"mod": mod, "name": name, **t}
    v = validate_one(Path(t["c_path"]), tag=f"reg_{prompts_dir.name}",
                     timeout=validate_timeout)
    return {"mod": mod, "name": name, **v}


def score_prompt(prompts_dir: Path, suite: list[tuple[str, str]],
                  out_dir: Path, model: str, api_key: str,
                  parallel: int = 4) -> dict:
    """Run the full suite, return aggregate stats."""
    results = []
    with cf.ThreadPoolExecutor(max_workers=parallel) as ex:
        futs = [ex.submit(score_routine, mod, name, prompts_dir, out_dir,
                          model, api_key)
                for mod, name in suite]
        for f in cf.as_completed(futs):
            r = f.result()
            sys.stderr.write(f"  [{r['mod']:8s}] {r['name']:30s} -> {r['status']}\n")
            sys.stderr.flush()
            results.append(r)
    pass_count = sum(1 for r in results if r["status"] == "pass")
    by_class = {}
    for r in results:
        by_class[r["status"]] = by_class.get(r["status"], 0) + 1
    return {
        "prompts_dir": str(prompts_dir),
        "suite_size": len(suite),
        "pass_count": pass_count,
        "pass_rate": pass_count / len(suite) if suite else 0,
        "by_class": by_class,
        "by_routine": results,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--prompts-dir", required=True, type=Path)
    ap.add_argument("--out-dir", required=True, type=Path,
                    help="where to write the translated .c files (per mod)")
    ap.add_argument("--json-out", type=Path, default=None,
                    help="path for the JSON score report")
    ap.add_argument("--model", default="gemma4:31b")
    ap.add_argument("--api-key", default=None)
    ap.add_argument("--parallel", type=int, default=4)
    args = ap.parse_args()

    api_key = args.api_key
    if api_key is None:
        kp = Path.home() / ".ollama" / "ff4-port.api.key"
        if kp.exists():
            api_key = kp.read_text().strip()
    if not api_key:
        sys.exit("no api key (--api-key or ~/.ollama/ff4-port.api.key)")

    sys.stderr.write(f"[regression] scoring {args.prompts_dir} against "
                     f"{len(DEFAULT_SUITE)} routines\n")
    result = score_prompt(args.prompts_dir, DEFAULT_SUITE, args.out_dir,
                          args.model, api_key, args.parallel)
    sys.stderr.write(
        f"[regression] PASS {result['pass_count']}/{result['suite_size']} "
        f"({result['pass_rate']:.0%})\n"
    )
    sys.stderr.write(f"[regression] by_class: {result['by_class']}\n")
    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(json.dumps(result, indent=2))
        sys.stderr.write(f"[regression] wrote {args.json_out}\n")
    else:
        print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
