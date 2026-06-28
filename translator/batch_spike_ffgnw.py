#!/usr/bin/env python3
"""batch_spike_ffgnw — run the parity spike against the LIVE ff4-gnw bodies.

Promotes L1 dispatches to L2 by proving per-routine runtime equivalence:
for every routine, generate_spike.py builds a fuzz harness that runs the
ported C vs the original asm (interpreter) on the same (fuzzed) entry state
and compares the observable output slot. fails==0 over N trials ⇒ L2.

Unlike generate_spike's original port/ tree, this points at the real
ff4-gnw/<module>/<Name>.c bodies that actually ship in the dispatch table.

Input  : a list of "ID Name_c domain" lines (the L1 set), via --routines FILE
         or stdin. Domain is the dispatch comment (battle/field/menu/...).
Output : JSONL results to --out (resumable: already-logged routines skipped),
         plus a running summary on stderr.

Outcomes per routine:
  pass         fails==0 over N trials                  → L2 candidate
  fail         fails>0 (real divergence)               → stays L1, investigate
  custom_spike generate_spike refused (indexed store)  → stays L1
  no_source    no standalone ff4-gnw/<dir>/<Name>.c    → stays L1 (bundled/odd)
  no_contract  file lacks a parseable CONTRACT block   → stays L1
  build_error  spike failed to compile                 → stays L1
  timeout      exceeded per-routine wall clock         → stays L1
"""
from __future__ import annotations
import argparse, json, re, subprocess, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent                       # ff4-port/
FFGNW = (ROOT.parent / "ff4-gnw")        # sibling submodule
GEN = HERE / "generate_spike.py"
# domain (dispatch comment) → ff4-gnw source subdir. btlgfx bodies are bundled
# in battle/btlgfx_*.c (many funcs per file) → not standalone-spikable here.
DOMAIN_DIR = {"battle": "battle", "field": "field", "menu": "menu",
              "cutscene": "cutscene", "sound": "sound"}
SUMMARY_RE = re.compile(r"trials:\s*(\d+),\s*fails:\s*(\d+)")


def find_source(name_c: str, domain: str) -> Path | None:
    """ff4-gnw source file for dispatch symbol Name_c (strip trailing _c)."""
    base = name_c[:-2] if name_c.endswith("_c") else name_c
    # Prefer the domain's dir, then any dir (a few routines sit cross-domain).
    dirs = [DOMAIN_DIR.get(domain, domain)] + list(DOMAIN_DIR.values())
    seen = set()
    for d in dirs:
        if d in seen:
            continue
        seen.add(d)
        cand = FFGNW / d / f"{base}.c"
        if cand.is_file():
            return cand
    return None


def classify(name_c: str, domain: str, trials: int, timeout: int,
             run_timeout: int) -> dict:
    src = find_source(name_c, domain)
    if src is None:
        return {"status": "no_source"}
    text = src.read_text(errors="replace")
    if "// CONTRACT:" not in text:
        return {"status": "no_contract", "file": str(src)}
    try:
        r = subprocess.run(
            [sys.executable, str(GEN), str(src), "--build", "--run", str(trials),
             "--run-timeout", str(run_timeout)],
            capture_output=True, text=True, timeout=timeout, cwd=str(ROOT))
    except subprocess.TimeoutExpired:
        return {"status": "timeout", "file": str(src)}
    out = (r.stdout or "") + "\n" + (r.stderr or "")
    if "CUSTOM_SPIKE" in out:
        return {"status": "custom_spike", "file": str(src)}
    if "delegate wrapper" in out:
        # Thin shim that calls run_emulated_func to its own asm → equivalent by
        # construction (it IS the interpreter path). No spike needed.
        return {"status": "delegate", "file": str(src)}
    m = SUMMARY_RE.search(out)
    if m:
        fails = int(m.group(2))
        return {"status": "pass" if fails == 0 else "fail",
                "trials": int(m.group(1)), "fails": fails, "file": str(src)}
    # No summary line → distinguish the failure mode.
    tail = "\n".join(out.strip().splitlines()[-4:])
    if "infinite loop" in out or "exceeded" in out:
        st = "run_hang"          # C body loops under fuzzed input (no per-trial guard)
    elif "Traceback" in out or "ValueError" in out or "invalid literal" in out:
        st = "parser_error"      # generate_spike couldn't parse the CONTRACT
    elif "error:" in out.lower() or "Error 1" in out:
        st = "compile_error"     # spike TU failed to compile
    else:
        st = "build_error"       # catch-all
    return {"status": st, "file": str(src), "tail": tail}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--routines", type=Path, help="file: 'ID Name_c domain' per line; default stdin")
    ap.add_argument("--out", type=Path, required=True, help="JSONL results (resumable)")
    ap.add_argument("--trials", type=int, default=200)
    ap.add_argument("--timeout", type=int, default=90, help="per-routine wall clock (s)")
    ap.add_argument("--run-timeout", type=int, default=20, help="spike-binary run budget passed to generate_spike (s)")
    args = ap.parse_args()

    lines = (args.routines.read_text() if args.routines else sys.stdin.read()).splitlines()
    todo = []
    for ln in lines:
        p = ln.split()
        if len(p) >= 3:
            todo.append((p[0], p[1], p[2]))   # id, name_c, domain

    done = {}
    if args.out.exists():
        for ln in args.out.read_text().splitlines():
            ln = ln.strip()
            if ln:
                o = json.loads(ln)
                done[o["id"]] = o["status"]

    from collections import Counter
    tally = Counter(done.values())
    with args.out.open("a") as f:
        for i, (rid, name_c, domain) in enumerate(todo, 1):
            if rid in done:
                continue
            res = classify(name_c, domain, args.trials, args.timeout, args.run_timeout)
            res = {"id": rid, "name": name_c, "domain": domain, **res}
            f.write(json.dumps(res) + "\n")
            f.flush()
            tally[res["status"]] += 1
            sys.stderr.write(f"[{i}/{len(todo)}] {rid} {name_c} → {res['status']}\n")
            sys.stderr.flush()
    sys.stderr.write(f"\n=== batch_spike summary === {dict(tally)}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
