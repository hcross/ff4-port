#!/usr/bin/env python3
"""qualify — one-command per-routine qualification: L1 (with a CONTRACT) -> L2.

Collapses the tribal command sequence (find the file, check it has a
CONTRACT, run generate_spike.py with the right flags, read the trial
count, hand-edit the registry) into one invocation with a typed per-stage
verdict, so a Sonnet-class agent's job becomes "run qualify, read the
failing stage's REASON, apply the ESCALATION.md remedy" instead of
improvising.

Scope (v1): L1 -> L2 only, for a routine whose CONTRACT already exists.
Writing a CONTRACT (needs an LLM call or a human) and L2 -> L3 oracle
validation (needs fixture selection, a judgment call) are explicitly out
of scope — see workflows/WF-DECOMP.md / WF-VALID.md for those steps.

Usage:
    python translator/qualify.py D<id> [--trials N] [--no-promote]

Exit code: 0 on a clean promotion, 1 on any escalation/blocked stage
(see the printed STAGE=... line for the reason), 2 on usage error.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent          # ff4-port/translator
PORT_ROOT = HERE.parent                          # ff4-port
UMBRELLA = PORT_ROOT.parent                      # ff4/ (env override: FF4_UMBRELLA_DIR)
import os
UMBRELLA = Path(os.environ.get("FF4_UMBRELLA_DIR", str(UMBRELLA)))
REGISTRY_DIR = UMBRELLA / "registry"
FFGNW = UMBRELLA / "ff4-gnw"
GEN_SPIKE = HERE / "generate_spike.py"

MODULE_DIRS = ["battle", "field", "menu", "cutscene", "sound", "btlgfx"]

# ESCALATION.md T-codes this stage machine can hit. Kept here as a single
# source so the printed REASON always matches the doc.
T2_CONTRACT_MMIO = "T2-contract-mmio: CONTRACT_MMIO_MISMATCH — declare mmio_effects/dma correctly, see prompts/reverser_system.md Pitfall 13/16"
T3_CUSTOM_SPIKE = "T3-harness-design: indexed store / no single-slot contract — needs a hand-written SPIKE_COMPARE/SPIKE_MASK, see generate_spike.py"
T1_DIVERGENCE = "T1-divergence: spike fails > 0 — a human/frontier session must read the failing trial against the asm"
T6_INFRA = "T6-infra: unexpected tool failure — diagnose, do not improvise a workaround"
T5_DO_NOT_DISPATCH = "T5-do-not-dispatch (terminal, not an escalation): WAIT_BLOCKING/TAIL_JML/SPC_MAILBOX flag — this routine is categorically excluded"


def emit(stage: str, status: str, reason: str = "") -> None:
    line = f"STAGE={stage} STATUS={status}"
    if reason:
        line += f" REASON={reason}"
    print(line)


def load_registry(registry_dir: Path) -> list[dict]:
    state = registry_dir / "dispatch_state.jsonl"
    records = []
    with state.open() as fh:
        for line in fh:
            line = line.strip()
            if line:
                records.append(json.loads(line))
    return records


def find_source(base_name: str) -> Path | None:
    for mod in MODULE_DIRS:
        cand = FFGNW / mod / f"{base_name}.c"
        if cand.is_file():
            return cand
    return None


_SUMMARY_RE = re.compile(r"trials:\s*(\d+),\s*fails:\s*(\d+)")


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("dispatch_id", help="e.g. D0081F4")
    ap.add_argument("--trials", type=int, default=200)
    ap.add_argument("--no-promote", action="store_true",
                     help="run the spike but don't call registry_promote.py")
    ap.add_argument("--registry-dir", type=Path, default=REGISTRY_DIR,
                     help="override for testing against an isolated registry copy")
    args = ap.parse_args(argv)

    if not args.registry_dir.is_dir():
        emit("resolve", "escalate", T6_INFRA + f" (registry dir not found: {args.registry_dir})")
        return 1
    records = load_registry(args.registry_dir)
    rec = next((r for r in records if r["id"] == args.dispatch_id), None)
    if rec is None:
        sys.stderr.write(f"error: no dispatch with id {args.dispatch_id!r}\n")
        return 2
    emit("resolve", "pass", f"{rec['name']} ({rec['module']}, {rec['level']})")

    if rec["level"] not in ("L0", "L1"):
        emit("locate_source", "skip", f"already {rec['level']} — this tool only does L1->L2")
        return 0

    # T5 only applies to a routine not yet proven — an already-L2/L3 dispatch
    # with a WAIT_BLOCKING-family flag (e.g. FieldMain_c, which legitimately
    # calls WaitFrame once per frame as normal end-of-loop sync) is NOT a
    # do-not-dispatch case; it already works. Checked after the level
    # early-return above, on purpose (ESCALATION.md's own calibration note).
    flags = rec.get("flags", [])
    if any(f in flags for f in ("WAIT_BLOCKING", "TAIL_JML", "SPC_MAILBOX")):
        emit("flag_check", "escalate", T5_DO_NOT_DISPATCH + f" (flags: {flags})")
        return 1
    emit("flag_check", "pass")

    base = rec["name"][:-2] if rec["name"].endswith("_c") else rec["name"]
    src = find_source(base)
    if src is None:
        emit("locate_source", "blocked",
             f"no ff4-gnw/<module>/{base}.c found — write the C translation first "
             "(WF-DECOMP.md steps 1-3)")
        return 1
    if "// CONTRACT:" not in src.read_text():
        emit("locate_source", "blocked",
             f"{src} has no CONTRACT block — write one before spiking "
             "(WF-DECOMP.md step 3, prompts/reverser_system.md format)")
        return 1
    emit("locate_source", "pass", str(src))

    # --spike-suffix qualify: generate_spike.py's own default ("auto") is the
    # same suffix ad-hoc manual runs use, so an unqualified run here would
    # silently overwrite whatever another session's parity/src/spike_<name>_auto.c
    # last generated. Give qualify.py its own namespace instead.
    try:
        res = subprocess.run(
            [sys.executable, str(GEN_SPIKE), str(src), "--build", "--run", str(args.trials),
             "--spike-suffix", "qualify"],
            capture_output=True, text=True, timeout=120, cwd=str(PORT_ROOT),
        )
    except subprocess.TimeoutExpired:
        emit("spike", "escalate", T6_INFRA + " (generate_spike.py timed out)")
        return 1
    out = (res.stdout or "") + "\n" + (res.stderr or "")

    if res.returncode == 3 or "CONTRACT_MMIO_MISMATCH" in out:
        emit("spike", "escalate", T2_CONTRACT_MMIO)
        return 1
    if "CUSTOM_SPIKE" in out:
        emit("spike", "escalate", T3_CUSTOM_SPIKE)
        return 1
    m = _SUMMARY_RE.search(out)
    if not m:
        tail = "\n".join(out.strip().splitlines()[-6:])
        emit("spike", "escalate", T6_INFRA + f" (no trial summary in output; tail: {tail!r})")
        return 1
    trials, fails = int(m.group(1)), int(m.group(2))
    if fails > 0:
        emit("spike", "escalate", T1_DIVERGENCE + f" ({fails}/{trials} fails)")
        return 1
    emit("spike", "pass", f"{trials}/{trials} pass")

    if args.no_promote:
        emit("promote", "skip", "--no-promote given")
        return 0

    evidence_dir = PORT_ROOT / "translator" / "runs" / "qualify_evidence"
    evidence_dir.mkdir(parents=True, exist_ok=True)
    evidence_path = evidence_dir / f"{args.dispatch_id}_{base}.txt"
    evidence_path.write_text(out)

    promote = subprocess.run(
        [sys.executable, str(args.registry_dir / "registry_promote.py"), args.dispatch_id,
         "--to", "L2", "--evidence", str(evidence_path),
         "--note", f"fuzzed spike, {trials}/{trials} pass (qualify.py)",
         "--state", str(args.registry_dir / "dispatch_state.jsonl")],
        capture_output=True, text=True,
    )
    if promote.returncode != 0:
        emit("promote", "escalate", T6_INFRA + f" (registry_promote.py: {promote.stderr.strip()})")
        return 1
    emit("promote", "pass", f"{args.dispatch_id} -> L2")
    return 0


if __name__ == "__main__":
    sys.exit(main())
