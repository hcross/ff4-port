"""Ca65BridgeBackend — implémentation REBackend pour disassemblies ca65/65816."""
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
# Classification ADR-003 — translate vs delegate
# ---------------------------------------------------------------------------

# Chain `lda <addr> / tax / stx <other>` qui consomme implicitement le
# registre B caché (high byte de l'accumulateur en mode A 8-bit). Indicateur
# fort de "composition non traduisible".
_B_HIDDEN_CHAIN_RE = re.compile(
    r"^\s*lda\s+\S+\s*(?:;[^\n]*)?\n"
    r"\s*(?:@\w+:\s*)?tax\s*(?:;[^\n]*)?\n"
    r"\s*(?:@\w+:\s*)?stx\s+\S+",
    re.MULTILINE,
)

# Heuristique : présence de `longa` explicite SANS `shorta`/`shorta0` final
# = fonction qui CHANGE le mode et le laisse 16-bit, pollue le caller.
# (Cas fréquent qui doit être délégué)
_LONGA_RE = re.compile(r"^\s*longa\b", re.MULTILINE)
_SHORTA_RE = re.compile(r"^\s*shorta0?\b", re.MULTILINE)


@dataclass
class Classification:
    """Décision de classification ADR-003 + signaux qui ont guidé."""
    decision: str            # 'translate' | 'delegate' | 'review'
    reasons: list[str]       # liste lisible des critères déclenchés


# Seuils ADR-003 (modifiable par config)
DEFAULT_TRANSLATE_INSTR_MAX = 50
DEFAULT_TRANSLATE_CALLS_MAX = 2


class Ca65BridgeBackend:
    """Backend RE pour projets de disassembly ca65/65816.

    Args:
        root: racine du repo de disassembly (contenant les modules `.asm`).
    """

    def __init__(self, root: Path | str):
        self.root = Path(root)
        if not self.root.is_dir():
            raise FileNotFoundError(f"root introuvable : {self.root}")

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
    # Cache de parsing (un parse complet ~80k LoC = ~100 ms typique)
    # ------------------------------------------------------------------

    @cached_property
    def _routines(self) -> dict:
        """Dict {label_name → Routine}. Parsé à la demande, mémoïsé."""
        return parse_tree(self.root)

    def _resolve(self, target: str):
        """Lookup d'une routine par label (préféré) ou par address hint."""
        if target in self._routines:
            return self._routines[target]
        # adresse hex acceptée : "0xC987" / "C987" / "$c987"
        norm = target.lower().lstrip("$").removeprefix("0x")
        for r in self._routines.values():
            if r.address_hint and r.address_hint == norm:
                return r
        return None

    # ------------------------------------------------------------------
    # REBackend protocol
    # ------------------------------------------------------------------

    def decompile(self, target: str):
        raise NotImplementedError("ca65-bridge ne fournit pas de pseudo-C")

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
            # On ignore les targets `@xxxx` (labels locaux internes) — ils
            # ne sont pas des xrefs au sens "vers une autre routine".
            if tgt.startswith("@"):
                continue
            # adresse approchée : address_hint de la target si disponible
            tgt_r = self._routines.get(tgt)
            addr = tgt_r.address_hint if tgt_r and tgt_r.address_hint else ""
            out.append(XRef(address=addr, name=tgt, ref_type=_classify_op(op)))
        return out

    def xrefs_to(self, target: str) -> list[XRef]:
        """Recherche les occurrences de `(jsr|jmp|bra...) <target>` dans tous les .asm."""
        results: list[XRef] = []
        # On recherche le motif "<op> <target>" ligne par ligne.
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
        return None  # non applicable

    def get_enum(self, name: str):
        return None  # non applicable

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
        # Pas de notion de "stub" dans le monde asm — toutes les fonctions
        # existent. Sémantique à raffiner en Phase 3.5 (registre des
        # traductions C/, ex. port/_translated.json).
        raise NotImplementedError(
            "ca65-bridge.unimplemented(): registre des traductions à implémenter "
            "en Phase 3.5"
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
    # ADR-003 — Classification translate vs delegate
    # ------------------------------------------------------------------

    def classify_routine(
        self,
        name: str,
        instr_max: int = DEFAULT_TRANSLATE_INSTR_MAX,
        calls_max: int = DEFAULT_TRANSLATE_CALLS_MAX,
    ) -> Classification | None:
        """Décide si une routine doit être traduite en C ou déléguée à l'asm.

        Critères ADR-003 (cumulables) :
          - instruction_count > instr_max → delegate (probable composition)
          - call_count > calls_max → delegate (multi-délégation, B caché)
          - présence de chain `lda/tax/stx` → delegate (Pitfall 9 confirmé)
          - présence de `longa` sans `shorta`/`shorta0` final → review

        Returns None si la routine est introuvable.
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
