"""Parser for ca65/65816 .asm files.

File model: a sequence of routines, each opened by a label at the start of
a line (`<Label>:`) and terminated by the next label or a comment
separator (`; ---...---`).

Primary target: the everything8215/ff4 repository (and other ca65/65816
projects that follow similar conventions).

Typical expected format:

    ; [ description ]

    CalcHits:
    @c987:  stz     $38fd       ; comment
            lda     $38fb
            beq     @c99e       ; return if no base hits
            ...
    @c99e:  rts

    ; ----------------------------------------------------

    CalcDmg:
    @c99f:  ...

Local labels (those prefixed with `@`) are **internal branch labels** and
do NOT define standalone routines — only root labels (no leading `@`) open
a new routine.

Parsing is intentionally **resilient**: rather than implementing a formal
ca65 tokenizer we rely on robust regexes that match this project's
conventions and most disassembly repos. Subclass or patch the regexes to
adapt to a different convention.
"""
from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

# Routine label: start of line, identifier, colon.
# No leading @, no leading whitespace.
_LABEL_RE = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*)\s*:\s*$")

# End-of-routine separator: a comment line of the form `; -------`.
_SEPARATOR_RE = re.compile(r"^\s*;\s*-{3,}\s*$")

# Instruction lines: we don't strictly validate 65816 mnemonics. We simply
# count non-blank, non-comment lines that are not labels.
_COMMENT_LINE_RE = re.compile(r"^\s*;")
_BLANK_LINE_RE = re.compile(r"^\s*$")

# Reference to another routine: control-transfer opcodes.
# Note: we also match the 65816 long jumps (jsl, jml).
_XREF_RE = re.compile(
    r"""
    ^\s*
    (?:@[A-Za-z0-9_]+:\s*)?         # optional local address (e.g. @c990:)
    (?P<op>jsr|jsl|jmp|jml|bra|brl|bcc|bcs|beq|bne|bpl|bmi|bvc|bvs)
    \s+
    (?P<target>[A-Za-z_][A-Za-z0-9_]*|@[A-Za-z0-9_]+)
    """,
    re.VERBOSE | re.IGNORECASE,
)

# 16-bit SNES address in a `@hhhh:` comment (ff4 disassembly convention)
_ADDR_HINT_RE = re.compile(r"@([0-9A-Fa-f]{4,6})\s*:")


@dataclass
class Routine:
    """A single isolated asm routine — label + verbatim body + metadata."""

    name: str
    file: Path
    line_start: int  # 1-based, line of the label
    line_end: int    # 1-based, last line of the body (inclusive)
    body: str        # verbatim content (label and trailing separator excluded)
    address_hint: str | None = None  # e.g. "c987" if found inside the body

    @property
    def instruction_count(self) -> int:
        """Count of lines that look like instructions (heuristic)."""
        return sum(
            1 for line in self.body.splitlines()
            if line.strip()
            and not _COMMENT_LINE_RE.match(line)
            and not _LABEL_RE.match(line)
        )

    @property
    def call_count(self) -> int:
        """Count of `jsr`/`jsl` (potentially external calls)."""
        return sum(
            1 for line in self.body.splitlines()
            if (m := _XREF_RE.search(line)) and m.group("op").lower() in ("jsr", "jsl")
        )

    def xrefs_out(self) -> list[tuple[str, str]]:
        """List of outgoing (op, target) — root and local labels together."""
        out: list[tuple[str, str]] = []
        for line in self.body.splitlines():
            m = _XREF_RE.search(line)
            if m:
                out.append((m.group("op").lower(), m.group("target")))
        return out


def _classify_op(op: str) -> str:
    op = op.lower()
    if op in ("jsr", "jsl"):
        return "call"
    if op in ("jmp", "jml"):
        return "jump"
    return "branch"  # bra, brl, bcc, bcs, beq, bne, bpl, bmi, bvc, bvs


def parse_file(path: Path) -> list[Routine]:
    """Parse a single .asm file and return its routines.

    A routine starts at `<Label>:` (root level, no `@` prefix) and extends
    until the next root label OR the next comment separator
    (`; ---...---`).
    """
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    routines: list[Routine] = []

    i = 0
    n = len(lines)
    while i < n:
        m = _LABEL_RE.match(lines[i])
        if not m:
            i += 1
            continue

        name = m.group(1)
        start = i
        # Advance until the next root label OR separator.
        j = i + 1
        body_lines: list[str] = []
        while j < n:
            if _LABEL_RE.match(lines[j]) or _SEPARATOR_RE.match(lines[j]):
                break
            body_lines.append(lines[j])
            j += 1

        # Trim trailing blank/comment lines (cosmetic).
        while body_lines and (_BLANK_LINE_RE.match(body_lines[-1])
                              or _COMMENT_LINE_RE.match(body_lines[-1])):
            body_lines.pop()

        body = "\n".join(body_lines)
        # First address hint inside the body (e.g. @c987:)
        addr_hint = None
        m_addr = _ADDR_HINT_RE.search(body)
        if m_addr:
            addr_hint = m_addr.group(1).lower()

        routines.append(Routine(
            name=name,
            file=path,
            line_start=start + 1,
            line_end=start + 1 + len(body_lines),
            body=body,
            address_hint=addr_hint,
        ))
        i = j

    return routines


def iter_asm_files(root: Path) -> list[Path]:
    """List the .asm files under root (recursive). Ignore `obj/` dirs."""
    return [
        p for p in sorted(root.rglob("*.asm"))
        if "obj" not in p.parts
    ]


def parse_tree(root: Path) -> dict[str, Routine]:
    """Parse the full .asm tree under root → dict {name: Routine}.

    If a name collides (two identical labels across files), the first
    occurrence wins. This happens in multi-version repos (e.g.
    title_en.asm vs title_jp.asm); to be refined in Phase 3.5.
    """
    routines: dict[str, Routine] = {}
    for asm in iter_asm_files(root):
        for r in parse_file(asm):
            routines.setdefault(r.name, r)
    return routines
