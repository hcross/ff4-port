"""Unit tests for apply_ips.py — synthetic patches over synthetic ROMs.

Run from ff4-port/: python3 -m pytest patches/tests/ -q
"""

import sys
import zlib
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from apply_ips import (  # noqa: E402
    IpsError,
    build_variant,
    detect_headered,
    lorom_pc,
    merge_ranges,
    next_pow2,
    parse_ips,
)

BANK = 0x8000


def ips(*chunks: bytes) -> bytes:
    return b"PATCH" + b"".join(chunks) + b"EOF"


def rec(off: int, payload: bytes) -> bytes:
    return off.to_bytes(3, "big") + len(payload).to_bytes(2, "big") + payload


def rle(off: int, run: int, byte: int) -> bytes:
    return off.to_bytes(3, "big") + b"\x00\x00" + run.to_bytes(2, "big") + bytes([byte])


def make_rom(size: int = 2 * BANK) -> bytes:
    """Deterministic filler with a sane LoROM header (reset vector, title)."""
    rom = bytearray((i * 7 + 3) & 0xFF for i in range(size))
    rom[0x7FC0:0x7FD5] = b"TEST ROM             "
    rom[0x7FFC:0x7FFE] = (0x8000).to_bytes(2, "little")
    return bytes(rom)


# --------------------------------------------------------------------------
# parse_ips
# --------------------------------------------------------------------------


def test_parse_simple_and_rle():
    records, trunc = parse_ips(ips(rec(0x10, b"AB"), rle(0x100, 5, 0x9A)))
    assert trunc is None
    assert records == [(0x10, b"AB"), (0x100, b"\x9a" * 5)]


def test_parse_truncate_extension():
    data = ips(rec(0, b"Z")) + (0x1234).to_bytes(3, "big")
    records, trunc = parse_ips(data)
    assert records == [(0, b"Z")]
    assert trunc == 0x1234


def test_parse_rejects_bad_magic():
    with pytest.raises(IpsError):
        parse_ips(b"NOPE" + b"EOF")


def test_parse_rejects_unterminated():
    with pytest.raises(IpsError):
        parse_ips(b"PATCH" + rec(0, b"Q"))


def test_zero_run_rle_is_noop():
    records, _ = parse_ips(ips(rle(0x20, 0, 0xFF), rec(0x30, b"K")))
    assert records == [(0x30, b"K")]


# --------------------------------------------------------------------------
# helpers
# --------------------------------------------------------------------------


def test_merge_ranges():
    assert merge_ranges([[10, 20], [20, 30], [40, 50], [45, 60], [0, 5]]) == [
        [0, 5], [10, 30], [40, 60],
    ]


def test_lorom_pc():
    assert lorom_pc(0x000000) == "$00:8000"
    assert lorom_pc(0x007FFF) == "$00:FFFF"
    assert lorom_pc(0x008000) == "$01:8000"
    assert lorom_pc(0x100000) == "$20:8000"  # expansion area keeps mapping linearly


def test_next_pow2():
    assert next_pow2(0x180000) == 0x200000
    assert next_pow2(0x100000) == 0x100000


def test_detect_headered():
    # classic signature: max_end 0x200 past a bank multiple
    records = [(0x180000 + 0x1FF, b"x")]
    assert detect_headered(4 * BANK, records) is True
    records = [(0x17FFFF, b"x")]  # ends exactly on a bank multiple
    assert detect_headered(4 * BANK, records) is False
    assert detect_headered(4 * BANK + 0x200, []) is True  # base itself headered
    records = [(0x100, b"x")]  # writes nowhere near a boundary: ambiguous
    assert detect_headered(4 * BANK, records) is None


# --------------------------------------------------------------------------
# build_variant end-to-end (tmp_path)
# --------------------------------------------------------------------------


def build(tmp_path, rom: bytes, patch: bytes, **kw):
    base = tmp_path / "base.sfc"
    ipsf = tmp_path / "p.ips"
    out = tmp_path / "out.sfc"
    base.write_bytes(rom)
    ipsf.write_bytes(patch)
    report = build_variant(base, ipsf, out, **kw)
    return out.read_bytes(), report


def test_unheadered_apply_and_ranges(tmp_path):
    rom = make_rom()
    out, report = build(tmp_path, rom, ips(rec(0x40, b"XY")), headered="no", pad="off")
    assert out[0x40:0x42] == b"XY"
    assert out[:0x40] == rom[:0x40] and out[0x42:] == rom[0x42:]
    assert report["modified_rom_ranges"] == [
        {"start": 0x40, "end_excl": 0x42, "snes": "$00:8040-$00:8041"}
    ]
    assert report["header_region_writes"] == []


def test_headered_strip_and_header_writes(tmp_path):
    rom = make_rom()
    patch = ips(rec(0x000, b"HDR"), rec(0x200 + 0x40, b"QQ"))
    out, report = build(tmp_path, rom, patch, headered="yes", pad="off")
    assert len(out) == len(rom)
    assert out[0x40:0x42] == b"QQ"  # header offset stripped
    assert report["header_region_writes"] == [[0, 3]]
    assert report["modified_rom_ranges"][0]["start"] == 0x40


def test_expansion_and_pow2_padding(tmp_path):
    rom = make_rom(2 * BANK)  # 64 KiB
    patch = ips(rec(3 * BANK - 1, b"\xAA"))  # grows image to 96 KiB (not pow2)
    out, report = build(tmp_path, rom, patch, headered="no", pad="auto")
    assert report["output"]["size_prepad"] == 3 * BANK
    assert len(out) == 4 * BANK  # padded to next power of 2
    assert out[3 * BANK - 1] == 0xAA
    assert out[3 * BANK :] == b"\x00" * BANK


def test_truncate_extension_applies(tmp_path):
    rom = make_rom(2 * BANK)
    patch = ips(rec(0, b"A")) + (BANK).to_bytes(3, "big")
    out, report = build(tmp_path, rom, patch, headered="no", pad="off")
    assert len(out) == BANK
    assert report["output"]["size_prepad"] == BANK


def test_base_crc_guard(tmp_path):
    rom = make_rom()
    with pytest.raises(IpsError, match="expected"):
        build(tmp_path, rom, ips(rec(0, b"A")), headered="no",
              expect_base_crc32="DEADBEEF")
    good = format(zlib.crc32(rom) & 0xFFFFFFFF, "08X")
    _, report = build(tmp_path, rom, ips(rec(0, b"A")), headered="no", pad="off",
                      expect_base_crc32=good)
    assert report["base"]["crc32"] == good


def test_reset_vector_warning(tmp_path):
    rom = make_rom()
    patch = ips(rec(0x7FFC, b"\x00\x40"))  # stomp reset vector below $8000
    _, report = build(tmp_path, rom, patch, headered="no", pad="off")
    assert any("reset vector" in w for w in report["warnings"])


def test_headered_auto_detection(tmp_path):
    rom = make_rom(4 * BANK)
    # writes ending exactly 0x200 past a bank multiple -> auto says headered
    patch = ips(rec(4 * BANK + 0x200 - 1, b"\xBB"))
    _, report = build(tmp_path, rom, patch, headered="auto", pad="off")
    assert report["headered"] is True
