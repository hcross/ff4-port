"""Ca65BridgeBackend — REBackend implementation for ca65/65816 disassemblies."""
from __future__ import annotations

import re
from dataclasses import dataclass
from functools import cached_property
from pathlib import Path

from ca65_bridge.models import (
    AsmResult,
    BackendCapabilities,
    FunctionEntry,
    XRef,
)
from ca65_bridge.parsers.asm import (
    _classify_op,
    iter_asm_files,
    parse_file,
    parse_tree,
)


# ---------------------------------------------------------------------------
# ADR-003 classification — translate vs delegate
# ---------------------------------------------------------------------------

# `lda <addr> / tax / stx <other>` chain that implicitly consumes the
# hidden register B (high byte of the accumulator in A 8-bit mode). Strong
# signal of a non-translatable composition routine.
_B_HIDDEN_CHAIN_RE = re.compile(
    r"^\s*lda\s+\S+\s*(?:;[^\n]*)?\n"
    r"\s*(?:@\w+:\s*)?tax\s*(?:;[^\n]*)?\n"
    r"\s*(?:@\w+:\s*)?stx\s+\S+",
    re.MULTILINE,
)

# Heuristic: explicit `longa` WITHOUT a final `shorta`/`shorta0` means the
# routine changes the mode and leaves it 16-bit, which pollutes the caller.
# Common reason to delegate.
_LONGA_RE = re.compile(r"^\s*longa\b", re.MULTILINE)
_SHORTA_RE = re.compile(r"^\s*shorta0?\b", re.MULTILINE)


@dataclass
class Classification:
    """ADR-003 classification decision plus the signals that triggered it."""
    decision: str            # 'translate' | 'delegate' | 'review'
    reasons: list[str]       # human-readable list of criteria that fired


# ADR-003 thresholds (configurable)
DEFAULT_TRANSLATE_INSTR_MAX = 50
DEFAULT_TRANSLATE_CALLS_MAX = 2


class Ca65BridgeBackend:
    """RE backend for ca65/65816 disassembly projects.

    Args:
        root: root of the disassembly repo (the directory that contains
              the per-module `.asm` files).
    """

    def __init__(self, root: Path | str):
        self.root = Path(root)
        if not self.root.is_dir():
            raise FileNotFoundError(f"root not found: {self.root}")

    @property
    def capabilities(self) -> BackendCapabilities:
        return BackendCapabilities(
            has_decompile=False,
            has_asm=True,
            has_structs=False,
            has_xrefs=True,
            has_search=True,
            has_enums=False,
        )

    # ------------------------------------------------------------------
    # Parse cache (full parse ~80k LoC = ~100 ms typical)
    # ------------------------------------------------------------------

    @cached_property
    def _routines(self) -> dict:
        """Dict {label_name → Routine}. Parsed on demand, memoised."""
        return parse_tree(self.root)

    def _resolve(self, target: str):
        """Look up a routine by label (preferred) or by address hint."""
        if target in self._routines:
            return self._routines[target]
        # Accept hex addresses: "0xC987" / "C987" / "$c987"
        norm = target.lower().lstrip("$").removeprefix("0x")
        for r in self._routines.values():
            if r.address_hint and r.address_hint == norm:
                return r
        return None

    # ------------------------------------------------------------------
    # REBackend protocol
    # ------------------------------------------------------------------

    def decompile(self, target: str):
        raise NotImplementedError("ca65-bridge does not provide pseudo-C")

    def get_asm(self, target: str) -> AsmResult | None:
        r = self._resolve(target)
        if not r:
            return None
        return AsmResult(
            address=r.address_hint or "",
            instructions=f"{r.name}:\n{r.body}",
            instruction_count=r.instruction_count,
            call_count=r.call_count,
            has_fp_sensitive=False,
        )

    def xrefs_from(self, target: str) -> list[XRef]:
        r = self._resolve(target)
        if not r:
            return []
        out: list[XRef] = []
        for op, tgt in r.xrefs_out():
            # Ignore `@xxxx` targets (internal local labels) — they are
            # not xrefs in the "to another routine" sense.
            if tgt.startswith("@"):
                continue
            # Best-effort address: the target's address_hint when known.
            tgt_r = self._routines.get(tgt)
            addr = tgt_r.address_hint if tgt_r and tgt_r.address_hint else ""
            out.append(XRef(address=addr, name=tgt, ref_type=_classify_op(op)))
        return out

    def xrefs_to(self, target: str) -> list[XRef]:
        """Find all `(jsr|jmp|bra...) <target>` occurrences across .asm files."""
        results: list[XRef] = []
        # We grep "<op> <target>" line by line.
        pat = re.compile(
            rf"^\s*(?:@[A-Za-z0-9_]+:\s*)?"
            rf"(jsr|jsl|jmp|jml|bra|brl|bcc|bcs|beq|bne|bpl|bmi|bvc|bvs)"
            rf"\s+{re.escape(target)}\b",
            re.IGNORECASE,
        )
        for asm in iter_asm_files(self.root):
            for routine in parse_file(asm):
                for line in routine.body.splitlines():
                    m = pat.search(line)
                    if m:
                        results.append(XRef(
                            address=routine.address_hint or "",
                            name=routine.name,
                            ref_type=_classify_op(m.group(1)),
                        ))
        return results

    def get_struct(self, name: str):
        return None  # not applicable

    def get_enum(self, name: str):
        return None  # not applicable

    def search(self, pattern: str) -> list[FunctionEntry]:
        pat = re.compile(pattern)
        out: list[FunctionEntry] = []
        for name, r in self._routines.items():
            if pat.search(name):
                out.append(FunctionEntry(
                    address=r.address_hint or "",
                    name=name,
                    class_name=r.file.parent.name,  # battle, menu, etc.
                ))
        return out

    def unimplemented(self, filter_pattern: str | None = None) -> list[FunctionEntry]:
        # No notion of a "stub" in the asm world — every function exists.
        # Semantics to be refined in Phase 3.5 (translation registry,
        # e.g. port/_translated.json).
        raise NotImplementedError(
            "ca65-bridge.unimplemented(): translation registry to be implemented "
            "in Phase 3.5"
        )

    def remaining(self, class_name: str | None = None) -> list[FunctionEntry]:
        out: list[FunctionEntry] = []
        for name, r in self._routines.items():
            if class_name and r.file.parent.name != class_name:
                continue
            out.append(FunctionEntry(
                address=r.address_hint or "",
                name=name,
                class_name=r.file.parent.name,
            ))
        return out

    # ------------------------------------------------------------------
    # ADR-003 — translate vs delegate classification
    # ------------------------------------------------------------------

    def classify_routine(
        self,
        name: str,
        instr_max: int = DEFAULT_TRANSLATE_INSTR_MAX,
        calls_max: int = DEFAULT_TRANSLATE_CALLS_MAX,
    ) -> Classification | None:
        """Decide whether a routine should be translated to C or delegated to asm.

        ADR-003 criteria (cumulative):
          - instruction_count > instr_max → delegate (likely a composition)
          - call_count > calls_max → delegate (multi-delegation, hidden B)
          - presence of an `lda/tax/stx` chain → delegate (Pitfall 9 confirmed)
          - `longa` without a final `shorta`/`shorta0` → review

        Returns None if the routine cannot be found.
        """
        r = self._resolve(name)
        if not r:
            return None

        reasons: list[str] = []

        if r.instruction_count > instr_max:
            reasons.append(f"instr_count={r.instruction_count} > {instr_max}")

        if r.call_count > calls_max:
            reasons.append(f"call_count={r.call_count} > {calls_max}")

        if _B_HIDDEN_CHAIN_RE.search(r.body):
            reasons.append("lda/tax/stx chain detected (Pitfall 9)")

        has_longa = bool(_LONGA_RE.search(r.body))
        has_shorta = bool(_SHORTA_RE.search(r.body))
        if has_longa and not has_shorta:
            reasons.append("longa without final shorta — caller mode pollution risk")

        if reasons:
            return Classification(decision="delegate", reasons=reasons)
        return Classification(decision="translate", reasons=["passes ADR-003 criteria"])
