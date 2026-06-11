"""Tests du parser asm contre battle/damage.asm de ff4."""
from __future__ import annotations

from pathlib import Path

import pytest

from ca65_bridge.backend import Ca65BridgeBackend
from ca65_bridge.parsers.asm import parse_file


HERE = Path(__file__).resolve().parent
UPSTREAM = HERE.parent.parent / "upstream"
DAMAGE_ASM = UPSTREAM / "battle" / "damage.asm"


pytestmark = pytest.mark.skipif(
    not DAMAGE_ASM.exists(),
    reason="upstream/battle/damage.asm absent — bring-up Phase 1 requis",
)


def test_parse_damage_returns_six_routines():
    routines = parse_file(DAMAGE_ASM)
    names = [r.name for r in routines]
    assert names == [
        "CalcHits",
        "CalcDmg",
        "ApplyDmgMult",
        "GetDmgPtr",
        "ApplyDmg",
        "ApplyAttackStatus",
    ]


def test_calc_hits_body_isolated():
    routines = {r.name: r for r in parse_file(DAMAGE_ASM)}
    ch = routines["CalcHits"]
    assert ch.address_hint == "c987"
    assert "stz" in ch.body
    assert "jsr     Rand99" in ch.body
    assert "rts" in ch.body
    # CalcHits ne doit PAS contenir le body de CalcDmg
    assert "CalcDmg" not in ch.body


def test_calc_hits_metrics():
    routines = {r.name: r for r in parse_file(DAMAGE_ASM)}
    ch = routines["CalcHits"]
    # 11 instructions : stz/lda/beq/tay/jsr/cmp/bcs/inc/dey/bne/rts
    assert ch.instruction_count == 11
    # 1 jsr Rand99
    assert ch.call_count == 1


def test_backend_get_asm_calchits():
    b = Ca65BridgeBackend(UPSTREAM)
    r = b.get_asm("CalcHits")
    assert r is not None
    assert r.address == "c987"
    assert r.instruction_count == 11
    assert r.call_count == 1
    assert "Rand99" in r.instructions


def test_backend_xrefs_from_calchits():
    b = Ca65BridgeBackend(UPSTREAM)
    refs = b.xrefs_from("CalcHits")
    names = {x.name for x in refs}
    assert names == {"Rand99"}
    assert all(x.ref_type == "call" for x in refs)


def test_backend_xrefs_to_calchits():
    """CalcHits est appelé par d'autres routines du module battle."""
    b = Ca65BridgeBackend(UPSTREAM)
    refs = b.xrefs_to("CalcHits")
    assert len(refs) >= 1, "CalcHits doit avoir au moins 1 caller"


def test_backend_search_calc():
    b = Ca65BridgeBackend(UPSTREAM)
    matches = b.search(r"^Calc")
    names = {m.name for m in matches}
    assert "CalcHits" in names
    assert "CalcDmg" in names


def test_backend_capabilities():
    b = Ca65BridgeBackend(UPSTREAM)
    caps = b.capabilities
    assert caps.has_decompile is False
    assert caps.has_asm is True
    assert caps.has_xrefs is True
    assert caps.has_search is True
    assert caps.has_structs is False
    assert caps.has_enums is False
