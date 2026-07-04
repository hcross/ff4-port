#!/usr/bin/env python3
"""generate_pitfalls — single source of truth for the "Known pitfalls (MUST
avoid)" section shared by reverser_system.md and reverser_hardcore.md.

Both prompt files carried an identical, hand-maintained copy of this
section. It forked silently: Pitfalls 13-17 (MMIO/DMA/CONTRACT/uncertainty)
were added to reverser_system.md and never back-ported to
reverser_hardcore.md, which stayed capped at Pitfall 12 until the drift was
found and fixed by hand (2026-07-04). pitfalls.yaml replaces both
hand-maintained copies; this script renders it into both files between
`<!-- PITFALLS:GENERATED:* -->` markers, the same generated-section
convention as `registry/render_registry.py`.

CAUTION — reverser_system.md is also the write target of
`translator/prompt_mutation_loop.py` (ADR-004): given a routine that fails
translation, it asks an LLM critic for a full replacement prompt and
adopts it wholesale if the regression suite holds. That loop already
invented a brand-new pitfall this way once (Pitfall 11, see
prompts/history/v1/manifest.json) rather than just rewording an existing
one. It is dormant (last run 2026-06-13, no CI/cron trigger — grep for
prompt_mutation_loop before assuming otherwise) but not retired: if it is
ever re-run and adopts a new version, run `--check` before `--write`. A
drift on reverser_system.md at that point most likely means the critic
touched the pitfalls section — fold the new/reworded pitfall into
pitfalls.yaml by hand first. `--write` overwrites the generated block
unconditionally and would silently discard the loop's improvement.

Usage:
    python3 prompts/generate_pitfalls.py [--check]
    python3 prompts/generate_pitfalls.py --write
    python3 prompts/generate_pitfalls.py --extract [--source reverser_hardcore.md]
        Rebuild pitfalls.yaml from a .md file's generated block. Recovery
        tool for when the .yaml is lost or suspected stale — NOT part of
        normal operation, which flows yaml -> md, not md -> yaml.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

import yaml

HERE = Path(__file__).resolve().parent
YAML_PATH = HERE / "pitfalls.yaml"
TARGET_FILES = [HERE / "reverser_system.md", HERE / "reverser_hardcore.md"]

SECTION_HEADER = "# Known pitfalls (MUST avoid)"
NEXT_HEADER = "# Output format"
PITFALLS_START = (
    "<!-- PITFALLS:GENERATED:START (source: prompts/pitfalls.yaml — "
    "do not hand-edit; run `python3 prompts/generate_pitfalls.py --write`) -->"
)
PITFALLS_END = "<!-- PITFALLS:GENERATED:END -->"
PITFALL_RE = re.compile(r"^## Pitfall (\d+) — (.+)$", re.MULTILINE)


class LiteralStr(str):
    """Marker subclass so the YAML dumper renders this value with `|` block style."""


def _literal_representer(dumper: yaml.Dumper, data: str):
    return dumper.represent_scalar("tag:yaml.org,2002:str", data, style="|")


yaml.add_representer(LiteralStr, _literal_representer)


def load_pitfalls() -> list[dict]:
    data = yaml.safe_load(YAML_PATH.read_text())
    pitfalls = data["pitfalls"]
    for p in pitfalls:
        p["body"] = p["body"].rstrip("\n")
    return pitfalls


def dump_pitfalls(pitfalls: list[dict]) -> str:
    header = (
        "# Single source of truth for the \"Known pitfalls (MUST avoid)\" section\n"
        "# shared by reverser_system.md and reverser_hardcore.md (W1-5). Rendered\n"
        "# into both files by generate_pitfalls.py --write; --check detects drift.\n"
        "#\n"
        "# reverser_system.md is ALSO the write target of\n"
        "# translator/prompt_mutation_loop.py (ADR-004, dormant since 2026-06-13\n"
        "# v2 — see prompts/history/). That loop can invent a brand-new pitfall or\n"
        "# reword an existing one on its own initiative (it added Pitfall 11 this\n"
        "# way). Run --check after any prompt_mutation_loop.py adoption and fold\n"
        "# any new/reworded pitfall back into this file BEFORE running --write —\n"
        "# --write overwrites the generated block unconditionally.\n"
    )
    payload = {
        "pitfalls": [
            {"id": p["id"], "title": p["title"], "body": LiteralStr(p["body"] + "\n")}
            for p in pitfalls
        ]
    }
    body = yaml.dump(payload, sort_keys=False, allow_unicode=True, width=1000)
    return header + body


def render_block(pitfalls: list[dict]) -> str:
    entries = "\n\n".join(
        f"## Pitfall {p['id']} — {p['title']}\n\n{p['body']}" for p in pitfalls
    )
    return f"{PITFALLS_START}\n\n{entries}\n\n{PITFALLS_END}"


def apply_block(text: str, block: str) -> str:
    if PITFALLS_START in text and PITFALLS_END in text:
        pattern = re.escape(PITFALLS_START) + r".*?" + re.escape(PITFALLS_END)
        return re.sub(pattern, block.replace("\\", "\\\\"), text, count=1, flags=re.DOTALL)
    # Bootstrap: no markers yet, replace the whole hand-written span between
    # the section header and the next top-level header.
    if SECTION_HEADER not in text or NEXT_HEADER not in text:
        raise ValueError(f"could not locate {SECTION_HEADER!r} / {NEXT_HEADER!r}")
    header_end = text.index(SECTION_HEADER) + len(SECTION_HEADER)
    next_start = text.index(NEXT_HEADER, header_end)
    return text[:header_end] + "\n\n" + block + "\n\n" + text[next_start:]


def extract_pitfalls(source: Path) -> list[dict]:
    text = source.read_text()
    if SECTION_HEADER not in text or NEXT_HEADER not in text:
        raise ValueError(f"{source}: could not locate the pitfalls section")
    start = text.index(SECTION_HEADER) + len(SECTION_HEADER)
    end = text.index(NEXT_HEADER, start)
    section = text[start:end]
    # Drop generated-block marker lines if extracting from an already-rendered file.
    section = "\n".join(
        line for line in section.splitlines()
        if line.strip() not in (PITFALLS_START, PITFALLS_END)
    )
    matches = list(PITFALL_RE.finditer(section))
    if not matches:
        raise ValueError(f"{source}: no '## Pitfall N — Title' headers found")
    pitfalls = []
    for i, m in enumerate(matches):
        body_start = m.end()
        body_end = matches[i + 1].start() if i + 1 < len(matches) else len(section)
        body = section[body_start:body_end].strip("\n")
        pitfalls.append({"id": int(m.group(1)), "title": m.group(2), "body": body})
    return pitfalls


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    mode = ap.add_mutually_exclusive_group()
    mode.add_argument("--check", action="store_true",
                       help="diff only, do not write; exit 1 on drift (default)")
    mode.add_argument("--write", action="store_true",
                       help="render pitfalls.yaml into both prompt files")
    mode.add_argument("--extract", action="store_true",
                       help="rebuild pitfalls.yaml from a .md file (recovery only)")
    ap.add_argument("--source", type=Path, default=HERE / "reverser_hardcore.md",
                     help="source file for --extract (default: reverser_hardcore.md, "
                          "the file NOT touched by prompt_mutation_loop.py)")
    args = ap.parse_args(argv)

    if args.extract:
        pitfalls = extract_pitfalls(args.source)
        YAML_PATH.write_text(dump_pitfalls(pitfalls))
        print(f"extracted {len(pitfalls)} pitfalls from {args.source} -> {YAML_PATH}")
        return 0

    if not YAML_PATH.is_file():
        sys.stderr.write(f"error: {YAML_PATH} not found (run --extract first)\n")
        return 2

    pitfalls = load_pitfalls()
    block = render_block(pitfalls)

    drift = False
    for target in TARGET_FILES:
        original = target.read_text()
        try:
            rendered = apply_block(original, block)
        except ValueError as exc:
            sys.stderr.write(f"error: {target}: {exc}\n")
            return 2
        if rendered == original:
            print(f"{target} already up to date")
            continue
        if args.write:
            target.write_text(rendered)
            print(f"updated {target}")
        else:
            drift = True
            sys.stderr.write(f"drift: {target} does not match {YAML_PATH}\n")

    if drift and not args.write:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
