#!/usr/bin/env python3
"""strict_audit — re-run every proven dispatch's spike in strict full-state mode.

F15 (KNOWN_FINDINGS.md) showed that a contract-only spike observes one thing
(one output slot, or the registers, or the WRAM) and can therefore credit L2 to
a body that leaves A/X/m/NZVC or a neighbouring byte different from the asm.
The dispatch hook restores no CPU state, so those differences reach the
interpreted caller. This tool regenerates each L2/L3 routine's spike under its
own `strict` suffix, builds them in parallel, runs them with
FF4_SPIKE_STRICT=1, and reports which fields diverge per routine.

It never touches the registry: a strict divergence is a triage item (fix the
body, or justify the field as dead with a `// STRICT:` line next to the
CONTRACT), not an automatic demotion. parity/Makefile is restored on exit, so
the audit leaves no `ff4-spike-strict-*` rules behind.

Usage:
    python translator/strict_audit.py [--levels L2,L3] [--trials 2000] [--jobs 8]
                                      [--only D038407,D038379] [--out DIR]

Output: <out>/strict_audit.jsonl (one record per dispatch) and
<out>/STRICT_AUDIT.md (summary table, worst first).
"""
from __future__ import annotations

import argparse
import concurrent.futures as cf
import json
import os
import re
import subprocess
import sys
from collections import Counter
from datetime import date
from pathlib import Path

HERE = Path(__file__).resolve().parent
PORT_ROOT = HERE.parent
UMBRELLA = Path(os.environ.get("FF4_UMBRELLA_DIR", str(PORT_ROOT.parent)))
FFGNW = UMBRELLA / "ff4-gnw"
STATE = UMBRELLA / "registry" / "dispatch_state.jsonl"
GEN_SPIKE = HERE / "generate_spike.py"
PARITY = PORT_ROOT / "parity"
MAKEFILE = PARITY / "Makefile"
ROM = PORT_ROOT / "upstream" / "rom" / "ff4-jp1.sfc"
MODULE_DIRS = ["battle", "field", "menu", "cutscene", "sound", "btlgfx"]
SUFFIX = "strict"

_SUMMARY_RE = re.compile(r"=== summary === trials:\s*(\d+),\s*fails:\s*(\d+)")
_STRICT_RE = re.compile(r"=== strict === trials:\s*(\d+),\s*strict_fails:\s*(\d+),\s*fields:(.*)")
_FIELD_RE = re.compile(r"(\w+)=(\$?[0-9A-Fa-f]+)")
_WROTE_RE = re.compile(r"\(target (ff4-spike-[\w-]+)\)")


def find_source(name: str) -> Path | None:
    base = name[:-2] if name.endswith("_c") else name
    for stem in (base, base + "Entry"):
        for mod in MODULE_DIRS:
            cand = FFGNW / mod / f"{stem}.c"
            if cand.is_file():
                return cand
    return None


def generate(rec: dict) -> dict:
    """Sequential stage: generate_spike.py appends Makefile rules, not parallel-safe."""
    out = {"id": rec["id"], "name": rec["name"], "module": rec["module"], "level": rec["level"]}
    src = find_source(rec["name"])
    if src is None:
        return {**out, "status": "no_source"}
    text = src.read_text()
    if "// CONTRACT:" not in text:
        return {**out, "status": "no_contract", "source": str(src.relative_to(UMBRELLA))}
    out["source"] = str(src.relative_to(UMBRELLA))
    res = subprocess.run([sys.executable, str(GEN_SPIKE), str(src), "--spike-suffix", SUFFIX,
                          "--target-addr", rec["pc"]],
                         capture_output=True, text=True, cwd=str(PORT_ROOT), timeout=120)
    log = (res.stdout or "") + (res.stderr or "")
    if res.returncode == 3 or "CONTRACT_MMIO_MISMATCH" in log:
        return {**out, "status": "contract_mmio"}
    m = _WROTE_RE.search(log)
    if not m:
        if "delegate wrapper" in log:
            return {**out, "status": "delegate"}
        if "CUSTOM_SPIKE" in log or "indexed store" in log:
            return {**out, "status": "custom_spike"}
        return {**out, "status": "gen_error", "detail": log.strip()[-300:]}
    return {**out, "status": "generated", "target": m.group(1)}


def build(targets: list[str], jobs: int) -> None:
    subprocess.run(["make", "-k", f"-j{jobs}", "-C", str(PARITY), *targets],
                   capture_output=True, text=True)


def _exec(binary: Path, trials: int, timeout: int, strict: bool):
    env = dict(os.environ)
    if strict:
        env["FF4_SPIKE_STRICT"] = "1"
    else:
        env.pop("FF4_SPIKE_STRICT", None)
    return subprocess.run([str(binary), str(ROM), str(trials)], capture_output=True,
                          text=True, timeout=timeout, env=env)


def run(rec: dict, trials: int, timeout: int) -> dict:
    binary = PARITY / rec["target"]
    if not binary.is_file():
        return {**rec, "status": "build_error"}
    try:
        res = _exec(binary, trials, timeout, True)
    except subprocess.TimeoutExpired:
        # Compound routines (the historical "run_hang" class) need far fewer
        # trials per budget, exactly as the 2026-06-28 batch re-ran them.
        trials = max(20, trials // 10)
        try:
            res = _exec(binary, trials, timeout * 3, True)
        except subprocess.TimeoutExpired:
            return {**rec, "status": "timeout"}
    s = _SUMMARY_RE.search(res.stdout)
    st = _STRICT_RE.search(res.stdout)
    if not s or not st:
        return {**rec, "status": "run_error", "detail": (res.stdout + res.stderr).strip()[-300:]}
    fields = {k: v for k, v in _FIELD_RE.findall(st.group(3))}
    first_wram = fields.pop("first_wram", None)
    sample = [l for l in res.stdout.splitlines() if l.startswith("strict ")][:2]
    plain_fails = None
    if int(s.group(2)):
        # Strict mode seeds undeclared registers with noise. If the plain
        # contract run passes, the failure is caused by that noise: the body
        # or the asm reads a register the CONTRACT does not declare.
        try:
            p = _SUMMARY_RE.search(_exec(binary, trials, timeout * 3, False).stdout)
            plain_fails = int(p.group(2)) if p else None
        except subprocess.TimeoutExpired:
            pass
    return {**rec, "status": "ran", "trials": int(s.group(1)), "plain_contract_fails": plain_fails,
            "contract_fails": int(s.group(2)), "strict_fails": int(st.group(2)),
            "fields": {k: int(v) for k, v in fields.items()},
            "first_wram": first_wram, "sample": sample}


def verdict(r: dict) -> str:
    if r.get("status") != "ran":
        return r.get("status", "?")
    if r["contract_fails"]:
        return "HIDDEN_REG_INPUT" if r.get("plain_contract_fails") == 0 else "CONTRACT_FAIL"
    return "strict_clean" if r["strict_fails"] == 0 else "STRICT_DIVERGENT"


def write_md(results: list[dict], path: Path, trials: int) -> None:
    by = Counter(verdict(r) for r in results)
    fieldtot = Counter()
    for r in results:
        if verdict(r) == "STRICT_DIVERGENT":
            fieldtot.update(r["fields"].keys())
    lines = [
        "# Strict full-state spike audit",
        "",
        f"Generated by `translator/strict_audit.py` on {date.today().isoformat()} "
        f"({trials} trials per routine, `FF4_SPIKE_STRICT=1`). See KNOWN_FINDINGS F15 "
        "and WF-DECOMP §4. A divergent field is a triage item — fix the body or justify "
        "the field as dead (`// STRICT:` line) — never an automatic demotion. "
        "`HIDDEN_REG_INPUT` = the contract compare fails only once undeclared registers "
        "carry noise: the routine reads a register its CONTRACT does not declare, so the "
        "original spike never fuzzed that input. `CONTRACT_FAIL` = fails even without noise.",
        "",
        "| Verdict | Count |", "|---|---|",
        *[f"| {k} | {v} |" for k, v in by.most_common()],
        "",
        "Routines diverging per field: " + ", ".join(f"`{k}` {v}" for k, v in fieldtot.most_common()),
        "",
        "| ID | Routine | Level | Verdict | strict fails | Fields (trials diverging) | first WRAM |",
        "|---|---|---|---|---|---|---|",
    ]
    order = {"CONTRACT_FAIL": 0, "HIDDEN_REG_INPUT": 1, "STRICT_DIVERGENT": 2}
    for r in sorted(results, key=lambda r: (order.get(verdict(r), 2), -r.get("strict_fails", 0), r["id"])):
        f = " ".join(f"{k}={v}" for k, v in sorted(r.get("fields", {}).items(), key=lambda kv: -kv[1]))
        lines.append(f"| {r['id']} | `{r['name']}` | {r['level']} | {verdict(r)} | "
                     f"{r.get('strict_fails', '')} | {f} | {r.get('first_wram') or ''} |")
    path.write_text("\n".join(lines) + "\n")


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--levels", default="L2,L3")
    ap.add_argument("--only", default="", help="comma-separated dispatch IDs")
    ap.add_argument("--trials", type=int, default=2000)
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--timeout", type=int, default=120)
    ap.add_argument("--out", type=Path, default=HERE / "runs" / "strict_audit")
    args = ap.parse_args(argv)

    levels = set(args.levels.split(","))
    only = set(filter(None, args.only.split(",")))
    recs = [json.loads(l) for l in STATE.read_text().splitlines() if l.strip()]
    recs = ([r for r in recs if r["id"] in only] if only
            else [r for r in recs if r["level"] in levels])

    mk_backup = MAKEFILE.read_text()
    try:
        gen = []
        for i, r in enumerate(recs, 1):
            g = generate(r)
            gen.append(g)
            print(f"[gen {i}/{len(recs)}] {r['id']} {r['name']}: {g['status']}", file=sys.stderr)
        targets = [g["target"] for g in gen if g["status"] == "generated"]
        print(f"[build] {len(targets)} spikes, -j{args.jobs}", file=sys.stderr)
        build(targets, args.jobs)
    finally:
        MAKEFILE.write_text(mk_backup)

    results = [g for g in gen if g["status"] != "generated"]
    todo = [g for g in gen if g["status"] == "generated"]
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for r in ex.map(lambda g: run(g, args.trials, args.timeout), todo):
            results.append(r)
            print(f"[run] {r['id']} {r['name']}: {verdict(r)} {r.get('fields', '')}", file=sys.stderr)

    args.out.mkdir(parents=True, exist_ok=True)
    with (args.out / "strict_audit.jsonl").open("w") as fh:
        for r in sorted(results, key=lambda r: r["id"]):
            fh.write(json.dumps(r) + "\n")
    write_md(results, args.out / "STRICT_AUDIT.md", args.trials)
    by = Counter(verdict(r) for r in results)
    print(" ".join(f"{k}={v}" for k, v in by.most_common()))
    return 0


if __name__ == "__main__":
    sys.exit(main())
