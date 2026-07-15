#!/usr/bin/env python3
"""apply_ips — deterministic IPS applier producing canonical ROM variants.

One language variant = one canonical pre-patched image file, produced HERE and
consumed verbatim by both the desktop harness (argv[1]) and the device build
(FrogFS / SD entry). There is deliberately no runtime --patch option anywhere:
a single byte path means a single CRC32 everywhere, which is what keys the
dispatch profile at ff4_init (see the translation-patch ADR).

Modes:
  explicit:  apply_ips.py BASE.sfc PATCH.ips -o OUT.sfc [options]
  manifest:  apply_ips.py --patch-id ID [--manifest MANIFEST] [--base PATH]
             Resolves patch file, header mode, padding and expected hashes
             from the manifest entry, then verifies everything it can.
             Exit 1 on any hash mismatch.

IPS format handled: "PATCH" magic, plain and RLE records, the truncation
extension (EOF followed by a 3-byte length), big-endian, 16 MiB addressing.
IPS records carry no source checksum — base validation is ours, via
--expect-base-crc32 / the manifest's base identity.

Header mode: many 90s-era patches (J2e among them) target a *headered* image
(512-byte copier header prepended). IPS has no read dependency, so applying
to a zero-filled prepended header is byte-exact; the header is stripped from
the output. --headered auto detects the classic signature (max patched end
== 0x200 past a 32 KiB multiple while the base is an unheadered multiple).

Stdlib only. Python 3.9+.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
import zlib
from pathlib import Path

HERE = Path(__file__).resolve().parent
DEFAULT_MANIFEST = HERE / "manifest.json"
COPIER_HEADER = 0x200
BANK = 0x8000  # LoROM bank payload size

# --------------------------------------------------------------------------
# IPS parsing / application
# --------------------------------------------------------------------------


class IpsError(ValueError):
    pass


def parse_ips(data: bytes):
    """Yield (offset, payload_bytes) records; return (records, truncate_len).

    RLE records are expanded to their payload here — patch files are small
    (hundreds of KiB) so materializing is fine and keeps application trivial.
    EOF ambiguity: a plain record needs >= 6 bytes, so b"EOF" with exactly 3
    bytes remaining is always the terminator; with exactly 6 remaining the
    truncation-extension reading is preferred (matches common tools).
    """
    if data[:5] != b"PATCH":
        raise IpsError("not an IPS file (missing PATCH magic)")
    records = []
    i = 5
    n = len(data)
    while True:
        rem = n - i
        if rem <= 0:
            raise IpsError("unterminated IPS (no EOF marker)")
        if data[i : i + 3] == b"EOF" and rem in (3, 6):
            truncate = int.from_bytes(data[i + 3 : i + 6], "big") if rem == 6 else None
            return records, truncate
        if rem < 6:
            raise IpsError(f"garbage at tail of IPS (offset {i}, {rem} bytes left)")
        off = int.from_bytes(data[i : i + 3], "big")
        size = int.from_bytes(data[i + 3 : i + 5], "big")
        i += 5
        if size == 0:  # RLE record
            if n - i < 3:
                raise IpsError(f"truncated RLE record at {i}")
            run = int.from_bytes(data[i : i + 2], "big")
            payload = bytes([data[i + 2]]) * run
            i += 3
        else:
            if n - i < size:
                raise IpsError(f"truncated record at {i} (want {size} bytes)")
            payload = data[i : i + size]
            i += size
        if payload:
            records.append((off, payload))


def apply_records(image: bytearray, records) -> list[list[int]]:
    """Apply records in file order; return merged half-open modified ranges."""
    ranges = []
    for off, payload in records:
        end = off + len(payload)
        if end > len(image):
            image.extend(b"\x00" * (end - len(image)))
        image[off:end] = payload
        ranges.append([off, end])
    return merge_ranges(ranges)


def merge_ranges(ranges: list[list[int]]) -> list[list[int]]:
    out: list[list[int]] = []
    for start, end in sorted(ranges):
        if out and start <= out[-1][1]:
            out[-1][1] = max(out[-1][1], end)
        else:
            out.append([start, end])
    return out


# --------------------------------------------------------------------------
# Helpers
# --------------------------------------------------------------------------


def crc32_hex(data: bytes) -> str:
    return format(zlib.crc32(data) & 0xFFFFFFFF, "08X")


def sha256_hex(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def lorom_pc(offset: int) -> str:
    """File offset of an unheadered LoROM image -> SNES PC display string."""
    bank = offset // BANK
    addr = 0x8000 | (offset % BANK)
    return f"${bank:02X}:{addr:04X}"


def next_pow2(n: int) -> int:
    p = 1
    while p < n:
        p <<= 1
    return p


def detect_headered(base_size: int, records) -> bool | None:
    """Classic copier-header signature; None when ambiguous.

    A headered-target patch applied over an unheadered multiple-of-32KiB base
    shows max_end sitting exactly 0x200 past a 32 KiB multiple (and typically
    writes below offset 0x200, into the header region itself).
    """
    if base_size % BANK == COPIER_HEADER:
        return True  # caller's base file already carries a header
    if base_size % BANK != 0:
        return None
    max_end = max((off + len(p) for off, p in records), default=0)
    if max_end % BANK == COPIER_HEADER:
        return True
    if max_end % BANK == 0:
        return False
    return None


def header_sanity(rom: bytes) -> list[str]:
    """Cheap LoROM internal-header checks; warnings only (LakeSnes scores,
    it does not enforce — but a failed reset vector means an unbootable
    image and deserves a loud line in the report)."""
    warnings = []
    if len(rom) < 0x8000:
        return ["image smaller than one LoROM bank"]
    reset = int.from_bytes(rom[0x7FFC:0x7FFE], "little")
    if reset < 0x8000:
        warnings.append(f"LoROM reset vector ${reset:04X} < $8000 — image likely unbootable")
    title = rom[0x7FC0:0x7FD5]
    printable = sum(1 for b in title if 0x20 <= b < 0x7F)
    if printable < len(title) // 2:
        warnings.append("internal title mostly non-printable — header may be damaged")
    return warnings


# --------------------------------------------------------------------------
# Core pipeline
# --------------------------------------------------------------------------


def build_variant(base_path: Path, ips_path: Path, out_path: Path, *,
                  headered: str = "auto", pad: str = "auto",
                  expect_base_crc32: str | None = None,
                  report_path: Path | None = None) -> dict:
    base = base_path.read_bytes()
    base_crc = crc32_hex(base)
    if expect_base_crc32 and base_crc != expect_base_crc32.upper():
        raise IpsError(
            f"base ROM {base_path} has CRC32 {base_crc}, expected {expect_base_crc32.upper()}"
        )

    ips_data = ips_path.read_bytes()
    records, truncate = parse_ips(ips_data)

    if headered == "auto":
        det = detect_headered(len(base), records)
        if det is None:
            raise IpsError("cannot auto-detect header mode; pass --headered yes|no")
        use_header = det
    else:
        use_header = headered == "yes"

    base_has_header = len(base) % BANK == COPIER_HEADER
    if base_has_header:
        image = bytearray(base)  # base already in the patch's headered space
    elif use_header:
        image = bytearray(b"\x00" * COPIER_HEADER + base)
    else:
        image = bytearray(base)

    modified = apply_records(image, records)
    if truncate is not None:
        del image[truncate:]

    strip = COPIER_HEADER if (use_header or base_has_header) else 0
    rom = bytes(image[strip:])
    size_prepad = len(rom)

    if pad == "auto":
        target = next_pow2(size_prepad) if size_prepad & (size_prepad - 1) else size_prepad
    elif pad == "off":
        target = size_prepad
    else:
        target = int(pad, 0)
        if target < size_prepad:
            raise IpsError(f"--pad {pad} smaller than patched ROM (0x{size_prepad:X})")
    rom = rom + b"\x00" * (target - size_prepad)

    warnings = header_sanity(rom)

    # Modified ranges, expressed in unheadered ROM space for the impact tool.
    rom_ranges, header_writes = [], []
    for start, end in modified:
        s, e = start - strip, end - strip
        if e <= 0:
            header_writes.append([start, end])
            continue
        if s < 0:
            header_writes.append([start, strip])
            s = 0
        rom_ranges.append({
            "start": s,
            "end_excl": e,
            "snes": f"{lorom_pc(s)}-{lorom_pc(e - 1)}",
        })

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(rom)

    report = {
        "generated_by": "patches/apply_ips.py",
        "patch": {"file": str(ips_path), "sha256": sha256_hex(ips_data),
                  "records": len(records), "truncate_ext": truncate},
        "base": {"file": str(base_path), "size": len(base), "crc32": base_crc},
        "headered": use_header or base_has_header,
        "modified_rom_ranges": rom_ranges,
        "header_region_writes": header_writes,
        "output": {
            "file": str(out_path),
            "size_prepad": size_prepad,
            "size": len(rom),
            "crc32": crc32_hex(rom),
            "sha256": sha256_hex(rom),
            "internal_title": rom[0x7FC0:0x7FD5].decode("ascii", "replace").rstrip(),
        },
        "warnings": warnings,
    }
    if report_path:
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(report, indent=2) + "\n")
    return report


# --------------------------------------------------------------------------
# Manifest mode
# --------------------------------------------------------------------------


def run_manifest_entry(manifest_path: Path, patch_id: str,
                       base_override: Path | None, out_override: Path | None) -> int:
    manifest = json.loads(manifest_path.read_text())
    entry = next((p for p in manifest["patches"] if p["id"] == patch_id), None)
    if entry is None:
        known = ", ".join(p["id"] for p in manifest["patches"])
        print(f"error: no patch id '{patch_id}' in {manifest_path} (known: {known})",
              file=sys.stderr)
        return 2
    identity = manifest["rom_identities"][entry["base"]]

    base_path = base_override or (manifest_path.parent / ".." / identity["file_hint"]).resolve()
    ips_path = (manifest_path.parent / entry["patch_file"]).resolve()
    out_path = out_override or (manifest_path.parent / "out" / entry["output"]["file"]).resolve()
    report_path = out_path.with_suffix(out_path.suffix + ".report.json")

    ips_sha = sha256_hex(ips_path.read_bytes())
    if ips_sha != entry["patch_sha256"]:
        print(f"error: {ips_path} sha256 {ips_sha} != manifest {entry['patch_sha256']}",
              file=sys.stderr)
        return 1

    report = build_variant(
        base_path, ips_path, out_path,
        headered="yes" if entry["headered"] else "no",
        pad=str(entry["pad_to"]),
        expect_base_crc32=identity["crc32"],
        report_path=report_path,
    )

    ok = True
    for field in ("crc32", "sha256", "size"):
        got, want = report["output"][field], entry["output"][field]
        if got != want:
            print(f"error: output {field} {got} != manifest {want}", file=sys.stderr)
            ok = False
    for w in report["warnings"]:
        print(f"warning: {w}", file=sys.stderr)
    if ok:
        print(f"{out_path} OK — crc32 {report['output']['crc32']}, "
              f"{report['output']['size_prepad']:#x} -> {report['output']['size']:#x} bytes, "
              f"report {report_path}")
    return 0 if ok else 1


# --------------------------------------------------------------------------


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("base", nargs="?", type=Path, help="base ROM (explicit mode)")
    ap.add_argument("ips", nargs="?", type=Path, help="IPS patch (explicit mode)")
    ap.add_argument("-o", "--out", type=Path, help="output image path")
    ap.add_argument("--report", type=Path, help="write JSON report here")
    ap.add_argument("--headered", choices=("auto", "yes", "no"), default="auto",
                    help="patch targets a 512-byte-headered image (default: auto)")
    ap.add_argument("--pad", default="auto",
                    help="'auto' (next power of 2), 'off', or explicit size (default: auto)")
    ap.add_argument("--expect-base-crc32", help="refuse to run on any other base")
    ap.add_argument("--patch-id", help="manifest mode: entry id to build & verify")
    ap.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    args = ap.parse_args(argv)

    try:
        if args.patch_id:
            return run_manifest_entry(args.manifest, args.patch_id, args.base, args.out)
        if not (args.base and args.ips and args.out):
            ap.error("explicit mode needs BASE, IPS and -o OUT (or use --patch-id)")
        report = build_variant(args.base, args.ips, args.out,
                               headered=args.headered, pad=args.pad,
                               expect_base_crc32=args.expect_base_crc32,
                               report_path=args.report)
        for w in report["warnings"]:
            print(f"warning: {w}", file=sys.stderr)
        print(f"{args.out} — crc32 {report['output']['crc32']}, "
              f"{report['output']['size_prepad']:#x} -> {report['output']['size']:#x} bytes")
        return 0
    except (IpsError, FileNotFoundError, KeyError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
