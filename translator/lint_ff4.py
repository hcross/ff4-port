#!/usr/bin/env python3
"""lint_ff4 — static failure-class linter over the ff4-gnw C tree.

Surfaces the bug classes that recur across F1-F12 (KNOWN_FINDINGS.md) and
the translator-prompt-lessons drawer, none of which were previously
lintable — each has bitten the project at least once and reached the
device in at least one case (CheckMenu's DP bug, the ExecBtlGfx hang).

Rules (each independent; a file can trigger more than one):
  R1 MMIO_IN_WRAM   — a hardware register ($2100-$21FF, $4200-$43FF)
                       written as `ram[0xADDR] = ...` / `write16(ram,
                       0xADDR, ...)` instead of via the bus
                       (snes_write/snes_writeBBus). Pitfall 13.
  R2 BLOCKING_LOOP   — a `while(1)`/`for(;;)` body with no
                       `run_emulated_func`/`snes_runCycles` call inside:
                       a dispatched routine that can hang the emulator
                       forever (WaitKeyDown_c is a live example).
  R3 SILENT_STUB     — a called `*_emu` helper resolves only to a weak
                       no-op, while a differently-cased variant of the
                       same logical name has a strong (real) definition
                       elsewhere — the naming schism that leaves working
                       Wait* delegates dead and every caller silently
                       skipping the wait.
  R4 HALLUCINATION   — uncertainty language ("assuming", "Placeholder",
                       "likely maps to", "treat as absolute", "outside
                       WRAM") in generated C: the model guessed. Pitfall 17.
  R5 LANGUAGE        — French text (accented characters or common French
                       stopwords) in a file that must be English-only —
                       this policy has broken twice already.

This pass only SURFACES findings (JSONL + human summary); it does not
auto-fix. Exit code: 0 if no findings, 1 if any.

Usage:
    python lint_ff4.py [--root PATH] [--out findings.jsonl] [--rules R1,R2,...]
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent                                          # ff4-port/
DEFAULT_FFGNW = Path(os.environ.get("FF4_GNW_DIR", str(ROOT.parent / "ff4-gnw")))

MODULES = ["battle", "field", "menu", "cutscene", "sound"]

ALL_RULES = ["R1", "R2", "R3", "R4", "R5"]


def strip_comments(text: str) -> str:
    """Remove /* */ and // comments so findings can't hide inside them."""
    no_block = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    no_line = re.sub(r"//[^\n]*", "", no_block)
    return no_line


def line_of(text: str, pos: int) -> int:
    return text.count("\n", 0, pos) + 1


# ---------------------------------------------------------------------------
# R1 — MMIO written as WRAM
# ---------------------------------------------------------------------------

_MMIO_ASSIGN_RE = re.compile(
    r"\bram\s*\[\s*0x([0-9A-Fa-f]{3,4})\s*\]\s*=(?!=)"
    r"|\bwrite16\s*\(\s*ram\s*,\s*0x([0-9A-Fa-f]{3,4})\s*,",
)


def _in_mmio_range(addr: int) -> bool:
    return 0x2100 <= addr <= 0x21FF or 0x4200 <= addr <= 0x43FF


def check_mmio_in_wram(path: Path, stripped: str) -> list[dict]:
    findings = []
    for m in _MMIO_ASSIGN_RE.finditer(stripped):
        hex_str = m.group(1) or m.group(2)
        addr = int(hex_str, 16)
        if not _in_mmio_range(addr):
            continue
        findings.append({
            "rule": "R1", "category": "MMIO_IN_WRAM", "file": str(path),
            "line": line_of(stripped, m.start()),
            "message": f"ram[0x{addr:04X}] written directly — hardware register in "
                       f"${addr:04X} range must go through snes_write/snes_writeBBus "
                       "(Pitfall 13), not the WRAM array.",
        })
    return findings


# ---------------------------------------------------------------------------
# R2 — blocking loop with no emulator-advancing call inside
# ---------------------------------------------------------------------------

_LOOP_OPEN_RE = re.compile(r"\b(?:while\s*\(\s*1\s*\)|for\s*\(\s*;;\s*\))\s*\{")
_ADVANCES_EMU_RE = re.compile(r"\b(?:run_emulated_func|snes_runCycles|snes_runFrameBounded)\b")


def _extract_brace_body(text: str, open_brace_pos: int) -> str:
    depth = 0
    i = open_brace_pos
    n = len(text)
    while i < n:
        c = text[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return text[open_brace_pos:i + 1]
        i += 1
    return text[open_brace_pos:]  # unterminated (shouldn't happen in valid C)


def check_blocking_loop(path: Path, stripped: str) -> list[dict]:
    findings = []
    for m in _LOOP_OPEN_RE.finditer(stripped):
        body = _extract_brace_body(stripped, m.end() - 1)
        if not _ADVANCES_EMU_RE.search(body):
            findings.append({
                "rule": "R2", "category": "BLOCKING_LOOP", "file": str(path),
                "line": line_of(stripped, m.start()),
                "message": "while(1)/for(;;) with no run_emulated_func/snes_runCycles/"
                           "snes_runFrameBounded inside — if the exit condition is never "
                           "met at runtime this hangs the emulator forever when dispatched.",
            })
    return findings


# ---------------------------------------------------------------------------
# R3 — *_emu naming schism (weak stub called, strong impl dead under another name)
# ---------------------------------------------------------------------------

_EMU_DEF_RE = re.compile(
    r"(__attribute__\(\(weak\)\)\s*)?(?:static\s+)?void\s+(\w+_emu)\s*\([^)]*\)\s*\{"
)
_EMU_CALL_RE = re.compile(r"\b(\w+_emu)\s*\(")


def normalize_emu_name(name: str) -> str:
    base = name[:-4] if name.endswith("_emu") else name
    snake = re.sub(r"(?<!^)(?=[A-Z])", "_", base)
    return snake.lower()


def collect_emu_definitions(ffgnw: Path) -> dict[str, list[tuple[str, Path, bool]]]:
    """normalized name -> [(raw_name, file, is_weak), ...] across the whole tree."""
    defs: dict[str, list[tuple[str, Path, bool]]] = {}
    files = [ffgnw / "ff4_helpers.c", ffgnw / "dispatch_all.c"]
    for mod in MODULES:
        files.extend(sorted((ffgnw / mod).glob("*.c")))
    for f in files:
        if not f.is_file():
            continue
        stripped = strip_comments(f.read_text(errors="replace"))
        for m in _EMU_DEF_RE.finditer(stripped):
            is_weak = m.group(1) is not None
            raw = m.group(2)
            defs.setdefault(normalize_emu_name(raw), []).append((raw, f, is_weak))
    return defs


def collect_emu_calls(ffgnw: Path) -> dict[str, list[tuple[Path, int]]]:
    """raw called name -> [(file, line), ...], excluding the file's own definition line."""
    calls: dict[str, list[tuple[Path, int]]] = {}
    for mod in MODULES:
        mod_dir = ffgnw / mod
        if not mod_dir.is_dir():
            continue
        for f in sorted(mod_dir.glob("*.c")):
            stripped = strip_comments(f.read_text(errors="replace"))
            def_names = {m.group(2) for m in _EMU_DEF_RE.finditer(stripped)}
            for m in _EMU_CALL_RE.finditer(stripped):
                name = m.group(1)
                if name in def_names:
                    continue  # the definition's own signature, not a call
                calls.setdefault(name, []).append((f, line_of(stripped, m.start())))
    return calls


def check_emu_schism(ffgnw: Path) -> list[dict]:
    findings: list[dict] = []
    defs = collect_emu_definitions(ffgnw)
    calls = collect_emu_calls(ffgnw)
    reported: set[str] = set()
    for called_name, sites in sorted(calls.items()):
        norm = normalize_emu_name(called_name)
        variants = defs.get(norm, [])
        called_defs = [v for v in variants if v[0] == called_name]
        called_is_strong = any(not w for (_, _, w) in called_defs)
        if called_is_strong:
            continue  # calling a real implementation — fine
        strong_elsewhere = [v for v in variants if v[0] != called_name and not v[2]]
        f0, l0 = sites[0]
        if strong_elsewhere:
            strong_name, strong_file, _ = strong_elsewhere[0]
            key = f"{called_name}->{strong_name}"
            if key in reported:
                continue
            reported.add(key)
            findings.append({
                "rule": "R3", "category": "SILENT_STUB", "file": str(f0), "line": l0,
                "message": f"'{called_name}' resolves to a weak no-op, but a strong "
                           f"implementation '{strong_name}' exists in "
                           f"{strong_file.relative_to(ffgnw)} under a different naming "
                           f"convention — every one of {len(sites)} caller(s) silently "
                           "skips the real behavior.",
                "call_sites": [f"{p.relative_to(ffgnw)}:{ln}" for p, ln in sites],
            })
        elif not called_defs:
            if called_name in reported:
                continue
            reported.add(called_name)
            findings.append({
                "rule": "R3", "category": "SILENT_STUB", "file": str(f0), "line": l0,
                "message": f"'{called_name}' has no definition anywhere (weak or strong) "
                           f"— {len(sites)} caller(s) reference an undefined helper.",
                "call_sites": [f"{p.relative_to(ffgnw)}:{ln}" for p, ln in sites],
            })
        # else: weak-only, no strong variant under any case — a deliberate no-op
        # stub with no evidence of a lost implementation; not flagged (low signal).
    return findings


# ---------------------------------------------------------------------------
# R4 — uncertainty markers (the model guessed)
# ---------------------------------------------------------------------------

_HALLUCINATION_RE = re.compile(
    r"\b(assuming|placeholder|likely maps to|treat as absolute|outside wram)\b",
    re.IGNORECASE,
)


def check_hallucination(path: Path, raw_text: str) -> list[dict]:
    findings = []
    for m in _HALLUCINATION_RE.finditer(raw_text):
        findings.append({
            "rule": "R4", "category": "HALLUCINATION", "file": str(path),
            "line": line_of(raw_text, m.start()),
            "message": f"uncertainty marker {m.group(1)!r} — Pitfall 17: this phrase "
                       "means the translator guessed at an unresolved symbol/address.",
        })
    return findings


# ---------------------------------------------------------------------------
# R5 — French text (English-only policy, broken twice already)
# ---------------------------------------------------------------------------

_ACCENT_RE = re.compile(r"[éèêëàâäùûüçôîïœ]", re.IGNORECASE)
_FRENCH_STOPWORDS = [
    "avec", "pour", "dans", "cette", "ceci", "donc", "après", "avant",
    "ainsi", "leur", "sans", "chaque", "toujours", "jamais", "aussi",
    "être", "sont", "était", "cela", "afin", "lorsque", "peut",
]
_FRENCH_STOPWORD_RE = re.compile(
    r"\b(" + "|".join(_FRENCH_STOPWORDS) + r")\b", re.IGNORECASE
)


def check_language(path: Path, raw_text: str) -> list[dict]:
    findings = []
    m = _ACCENT_RE.search(raw_text)
    if m:
        findings.append({
            "rule": "R5", "category": "LANGUAGE", "file": str(path),
            "line": line_of(raw_text, m.start()),
            "message": f"accented character {m.group(0)!r} — English-only policy "
                       "(this has broken twice already, see AGENTS.md).",
        })
    m = _FRENCH_STOPWORD_RE.search(raw_text)
    if m:
        findings.append({
            "rule": "R5", "category": "LANGUAGE", "file": str(path),
            "line": line_of(raw_text, m.start()),
            "message": f"French stopword {m.group(0)!r} — English-only policy.",
        })
    return findings


# ---------------------------------------------------------------------------
# Driver
# ---------------------------------------------------------------------------

def lint_file(path: Path, rules: set[str]) -> list[dict]:
    raw = path.read_text(errors="replace")
    stripped = strip_comments(raw)
    findings: list[dict] = []
    if "R1" in rules:
        findings += check_mmio_in_wram(path, stripped)
    if "R2" in rules:
        findings += check_blocking_loop(path, stripped)
    if "R4" in rules:
        findings += check_hallucination(path, raw)
    if "R5" in rules:
        findings += check_language(path, raw)
    return findings


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", type=Path, default=DEFAULT_FFGNW,
                    help="ff4-gnw tree to lint (default: sibling ff4-gnw, "
                         "override with FF4_GNW_DIR env var)")
    ap.add_argument("--out", type=Path, help="write findings as JSONL to this path")
    ap.add_argument("--rules", default=",".join(ALL_RULES),
                    help=f"comma-separated subset of {ALL_RULES} (default: all)")
    ap.add_argument("--quiet", action="store_true", help="suppress the human summary")
    args = ap.parse_args(argv)

    if not args.root.is_dir():
        sys.stderr.write(f"error: ff4-gnw root not found: {args.root}\n")
        return 2

    rules = set(r.strip().upper() for r in args.rules.split(",") if r.strip())
    unknown = rules - set(ALL_RULES)
    if unknown:
        sys.stderr.write(f"error: unknown rule(s) {sorted(unknown)}; choices are {ALL_RULES}\n")
        return 2

    findings: list[dict] = []
    for mod in MODULES:
        mod_dir = args.root / mod
        if not mod_dir.is_dir():
            continue
        for f in sorted(mod_dir.glob("*.c")):
            findings += lint_file(f, rules)
    # Top-level files worth checking for R4/R5 too (not R1/R2 — they're
    # infra, not translated routine bodies).
    for name in ("ff4_helpers.c", "main.c"):
        f = args.root / name
        if f.is_file():
            raw = f.read_text(errors="replace")
            if "R4" in rules:
                findings += check_hallucination(f, raw)
            if "R5" in rules:
                findings += check_language(f, raw)

    if "R3" in rules:
        findings += check_emu_schism(args.root)

    findings.sort(key=lambda d: (d["file"], d.get("line", 0)))

    if args.out:
        with args.out.open("w") as fh:
            for d in findings:
                fh.write(json.dumps(d) + "\n")

    if not args.quiet:
        by_cat: dict[str, int] = {}
        for d in findings:
            by_cat[d["category"]] = by_cat.get(d["category"], 0) + 1
        print(f"lint_ff4: {len(findings)} finding(s) over {args.root}")
        for cat, n in sorted(by_cat.items()):
            print(f"  {cat}: {n}")
        for d in findings:
            rel = d["file"]
            try:
                rel = str(Path(d["file"]).relative_to(args.root))
            except ValueError:
                pass
            print(f"  [{d['category']}] {rel}:{d.get('line', '?')} — {d['message']}")

    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
