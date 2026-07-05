#!/usr/bin/env python3
"""select_exemplars — auto-curated few-shot exemplars for the LLM
translation prompt (Wave 3 / Part B of the 2026-07-03 acceleration audit).

Given the module + dispatch flags of a routine about to be translated,
picks the most similar ALREADY-VALIDATED (L2/L3/L4) routines from
registry/dispatch_state.jsonl as few-shot examples — a dynamic supplement
to prompts/reverser_examples.md's 2 static, hand-picked examples (which
stay untouched; this script never reads or edits that file, it only
prints an additional block for a caller to append).

Rule-based, not learned — matches this project's existing precedent
(classify_flags.py, the ADR-003 translate/delegate classifier): score =
Jaccard(target_flags, candidate_flags) + 0.5 if same module. No new
analysis pipeline and no persisted manifest: flags and level come
straight from the registry, re-read fresh on every run, so a routine
demoted after being an exemplar (e.g. ExecInterrupt_c's 2026-07-03
L2->L1 demotion) simply drops out next time — nothing to invalidate by
hand, nothing to go stale.

v1 scope, deliberately minimal (see the Wave 3 planning notes for what's
explicitly deferred to a later session): flag-overlap + module-match
scoring only, no instruction-count bucketing, wired into
batch_translate.py only (not cascade_translate.py / hardcore_translate.py
yet). The rendered block shows each exemplar's validated C body only —
NOT paired with the original asm the way reverser_examples.md's static
examples are (that would need a ca65-bridge round-trip; deferred).

Usage:
    python3 select_exemplars.py --module battle --flags DP_SENSITIVE,DMA_TRIGGER
    python3 select_exemplars.py --routine D008302   # module/flags looked up from the registry
    python3 select_exemplars.py --check             # sanity: candidates resolve to real source files
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent          # ff4-port/translator
PORT_ROOT = HERE.parent                         # ff4-port
ROOT = PORT_ROOT.parent                         # ff4 (umbrella)
REGISTRY = ROOT / "registry" / "dispatch_state.jsonl"
FF4GNW = ROOT / "ff4-gnw"

GOOD_LEVELS = {"L2", "L3", "L4"}


def load_records() -> list[dict]:
    records = []
    with REGISTRY.open() as fh:
        for line in fh:
            line = line.strip()
            if line:
                records.append(json.loads(line))
    return records


def load_candidates() -> list[dict]:
    return [r for r in load_records() if r.get("level") in GOOD_LEVELS]


def resolve_source(record: dict) -> Path | None:
    """Best-effort: find the .c file implementing this routine. Returns
    None for routines this script can't confidently locate (bundled
    files, name/filename mismatches) rather than guessing wrong."""
    module_dir = FF4GNW / record["module"]
    if not module_dir.is_dir():
        return None
    name = record["name"]
    stem = name[:-2] if name.endswith("_c") else name
    guess = module_dir / f"{stem}.c"
    if guess.is_file():
        return guess
    needle = f"{name}(Snes"
    for c_file in sorted(module_dir.glob("*.c")):
        try:
            if needle in c_file.read_text():
                return c_file
        except (UnicodeDecodeError, OSError):
            continue
    return None


def jaccard(a: list[str], b: list[str]) -> float:
    sa, sb = set(a), set(b)
    if not sa and not sb:
        return 0.0
    return len(sa & sb) / len(sa | sb)


def score(target_module: str, target_flags: list[str], candidate: dict) -> float:
    s = jaccard(target_flags, candidate.get("flags", []))
    if candidate["module"] == target_module:
        s += 0.5
    return s


def select(target_module: str, target_flags: list[str], n: int = 2,
           exclude_id: str | None = None) -> list[tuple[dict, Path]]:
    candidates = [c for c in load_candidates() if c["id"] != exclude_id]
    ranked = sorted(candidates, key=lambda c: score(target_module, target_flags, c),
                     reverse=True)
    picked: list[tuple[dict, Path]] = []
    for c in ranked:
        src = resolve_source(c)
        if src is None:
            continue
        picked.append((c, src))
        if len(picked) >= n:
            break
    return picked


def render_block(picked: list[tuple[dict, Path]]) -> str:
    parts = []
    for record, src in picked:
        flags = ", ".join(record.get("flags", [])) or "none"
        body = src.read_text().strip()
        parts.append(
            f"### Exemplar: {record['name']} ({record['module']}, "
            f"{record['addr_display']}, level {record['level']}, flags: {flags})\n\n"
            f"```c\n{body}\n```"
        )
    return "\n\n".join(parts)


def cmd_check() -> int:
    candidates = load_candidates()
    if not candidates:
        sys.stderr.write("error: no L2+ candidates found in the registry\n")
        return 1
    resolved = 0
    unresolved: list[str] = []
    for c in candidates:
        if resolve_source(c) is not None:
            resolved += 1
        else:
            unresolved.append(f"{c['id']} {c['name']} ({c['module']})")
    print(f"{resolved}/{len(candidates)} L2+ candidates resolve to a source file")
    if unresolved:
        print(f"unresolved (invisible to the exemplar bank, {len(unresolved)}):")
        for line in unresolved[:20]:
            print(f"  {line}")
        if len(unresolved) > 20:
            print(f"  ... and {len(unresolved) - 20} more")
    if resolved == 0:
        sys.stderr.write("error: zero candidates resolved — exemplar bank would be empty\n")
        return 1
    return 0


def cmd_select(args: argparse.Namespace) -> int:
    if args.routine:
        records = {r["id"]: r for r in load_records()}
        target = records.get(args.routine)
        if target is None:
            sys.stderr.write(f"error: {args.routine} not found in the registry\n")
            return 2
        module = target["module"]
        flags = target.get("flags", [])
        exclude_id = args.routine
    else:
        if not args.module:
            sys.stderr.write("error: --module is required unless --routine is given\n")
            return 2
        module = args.module
        flags = args.flags.split(",") if args.flags else []
        exclude_id = None

    picked = select(module, flags, n=args.n, exclude_id=exclude_id)
    if not picked:
        sys.stderr.write("warning: no exemplars could be selected/resolved\n")
        return 0
    print(render_block(picked))
    return 0


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--module", help="target routine's module (e.g. battle)")
    ap.add_argument("--flags", help="comma-separated dispatch flags of the target routine")
    ap.add_argument("--routine", help="dispatch ID to look up module/flags from the registry")
    ap.add_argument("-n", type=int, default=2, help="number of exemplars to select (default 2)")
    ap.add_argument("--check", action="store_true",
                     help="sanity-check that L2+ candidates resolve to real source files")
    args = ap.parse_args(argv)

    if args.check:
        return cmd_check()
    return cmd_select(args)


if __name__ == "__main__":
    sys.exit(main())
