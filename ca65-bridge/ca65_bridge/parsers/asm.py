"""Parser pour fichiers .asm ca65/65816.

Modèle d'un fichier : suite de routines, chacune ouverte par un label en début
de ligne (`<Label>:`) et terminée par le label suivant ou un séparateur de
commentaires (`; ---...---`).

Cible : repo everything8215/ff4 (et compatibles ca65/65816).

Format attendu typique :

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

Les labels locaux (préfixés `@`) sont des **labels de branche interne** et ne
constituent PAS des routines distinctes — uniquement les labels racine
(sans `@`) ouvrent une nouvelle routine.

Le parsing est volontairement **résilient** : pas de tokenizer formel ca65,
juste des regex robustes sur les conventions de ce projet et de la plupart des
repos de disassembly. Si un projet utilise une convention différente,
sous-classer ou patcher les regex.
"""
from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

# Label de routine : début de ligne, identifier, deux-points.
# Pas de leading @, pas de leading whitespace.
_LABEL_RE = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*)\s*:\s*$")

# Séparateur de fin de routine : ligne de commentaire `; -------`.
_SEPARATOR_RE = re.compile(r"^\s*;\s*-{3,}\s*$")

# Instruction line : trace d'au moins un mnémonique 65816 ; on ne valide pas
# strictement, on compte simplement les lignes non-commentaire non-vides qui
# ne sont pas un label.
_COMMENT_LINE_RE = re.compile(r"^\s*;")
_BLANK_LINE_RE = re.compile(r"^\s*$")

# Référence vers une autre routine : opcodes de transfert de contrôle.
# Note : on capture aussi les long jumps (jsl, jml) propres au 65816.
_XREF_RE = re.compile(
    r"""
    ^\s*
    (?:@[A-Za-z0-9_]+:\s*)?         # adresse locale optionnelle (ex. @c990:)
    (?P<op>jsr|jsl|jmp|jml|bra|brl|bcc|bcs|beq|bne|bpl|bmi|bvc|bvs)
    \s+
    (?P<target>[A-Za-z_][A-Za-z0-9_]*|@[A-Za-z0-9_]+)
    """,
    re.VERBOSE | re.IGNORECASE,
)

# Adresse SNES 16-bit en commentaire `@hhhh:` (convention ff4 disasm)
_ADDR_HINT_RE = re.compile(r"@([0-9A-Fa-f]{4,6})\s*:")


@dataclass
class Routine:
    """Une routine asm isolée — label + body verbatim + métadonnées."""

    name: str
    file: Path
    line_start: int  # 1-based, ligne du label
    line_end: int    # 1-based, dernière ligne du body (inclusive)
    body: str        # contenu verbatim (sans le label, sans le séparateur final)
    address_hint: str | None = None  # ex. "c987" si trouvé dans le body

    @property
    def instruction_count(self) -> int:
        """Compte des lignes qui ressemblent à des instructions (heuristique)."""
        return sum(
            1 for line in self.body.splitlines()
            if line.strip()
            and not _COMMENT_LINE_RE.match(line)
            and not _LABEL_RE.match(line)
        )

    @property
    def call_count(self) -> int:
        """Compte des `jsr`/`jsl` (calls externes potentiels)."""
        return sum(
            1 for line in self.body.splitlines()
            if (m := _XREF_RE.search(line)) and m.group("op").lower() in ("jsr", "jsl")
        )

    def xrefs_out(self) -> list[tuple[str, str]]:
        """Liste des (op, target) sortants — labels racine et locaux confondus."""
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
    """Parse un fichier .asm et retourne ses routines.

    Une routine commence à `<Label>:` (au niveau racine, non préfixé `@`) et
    s'étend jusqu'au prochain label racine OU au prochain séparateur de
    commentaires (`; ---...---`).
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
        # Avance jusqu'au prochain label racine OU séparateur.
        j = i + 1
        body_lines: list[str] = []
        while j < n:
            if _LABEL_RE.match(lines[j]) or _SEPARATOR_RE.match(lines[j]):
                break
            body_lines.append(lines[j])
            j += 1

        # Trim trailing blank/comment lines (cosmétique)
        while body_lines and (_BLANK_LINE_RE.match(body_lines[-1])
                              or _COMMENT_LINE_RE.match(body_lines[-1])):
            body_lines.pop()

        body = "\n".join(body_lines)
        # Premier indice d'adresse dans le body (ex. @c987:)
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
    """Liste les fichiers .asm sous root (récursif). Ignore les `obj/`."""
    return [
        p for p in sorted(root.rglob("*.asm"))
        if "obj" not in p.parts
    ]


def parse_tree(root: Path) -> dict[str, Routine]:
    """Parse tout l'arbre .asm sous root → dict {nom: Routine}.

    En cas de collision de nom (deux labels identiques dans deux fichiers),
    la première occurrence gagne. Cette situation arrive dans les repos
    multi-version (ex. title_en.asm vs title_jp.asm) ; à raffiner en
    Phase 3.5.
    """
    routines: dict[str, Routine] = {}
    for asm in iter_asm_files(root):
        for r in parse_file(asm):
            routines.setdefault(r.name, r)
    return routines
