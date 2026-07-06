#!/usr/bin/env python3
"""Auto-generate a parity spike harness for a translated function.

Input:
    port/<module>/<name>.c    — LLM-produced C translation containing a
                                 `// CONTRACT:` block (see prompts/reverser_system.md).

Output:
    parity/src/spike_<name>_auto.c — fuzz harness comparing asm vs C.
    Updates parity/Makefile with the new build target.

Then optionally builds it (`--build`) and runs N trials (`--run N`).

Usage:
    python translator/generate_spike.py port/battle/GetAIScriptPtr.c \\
        --module battle --build --run 100

The generated spike has the same skeleton as the M2/M3/M4 manual spikes:
load ROM, run 60 boot frames, snapshot baseline, fuzz N trials,
run_emulated_func on the asm address, run the C function on the same
WRAM state, compare a single observable RAM slot.
"""
from __future__ import annotations

import argparse
import dataclasses
import re
import subprocess
import sys
from pathlib import Path
from typing import Optional

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
PORT_DIR = ROOT / "port"
PARITY_SRC = ROOT / "parity" / "src"
PARITY_MAKEFILE = ROOT / "parity" / "Makefile"
BRIDGE_BIN = ROOT / "ca65-bridge" / ".venv" / "bin" / "ca65-bridge"
UPSTREAM = ROOT / "upstream"

# Module → SNES bank (must match translator/batch_translate.py MODULE_BANK).
MODULE_BANK = {
    "battle":   0x03,
    "btlgfx":   0x02,
    "menu":     0x01,
    "field":    0x00,
    "sound":    0x04,
    "cutscene": 0x13,
}

# Indexed-store opcode: `sta/stx/sty/stz $XXXX,x` or `,y`.
# When present, the output address is dynamic — auto-spike must declare
# CUSTOM_SPIKE: yes to avoid a vacuous comparison (where both implementations
# write to an unobserved address and the harness reads a third, unchanged one).
_INDEXED_STORE_RE = re.compile(
    r"^\s*(?:@[A-Za-z0-9_]+:\s*)?(?:sta|stx|sty|stz)\s+[^\s,]+,[xy]\b",
    re.IGNORECASE | re.MULTILINE,
)

# Hardware-register symbol table (Pitfall 13/14): the disassembly names MMIO
# registers symbolically (`hINIDISP`, `hMDMAEN`, ...), not by raw hex address
# — `upstream/include/hardware.inc` is the canonical `hSYMBOL := $addr` list.
_HW_INC = UPSTREAM / "include" / "hardware.inc"
_HW_SYMBOL_DEF_RE = re.compile(r"^\s*(h[A-Za-z0-9_]+)\s*:=\s*\$([0-9A-Fa-f]+)", re.MULTILINE)
_hw_symbols_cache: Optional[dict[str, int]] = None


def hw_symbols() -> dict[str, int]:
    """symbol name -> address, parsed once from hardware.inc."""
    global _hw_symbols_cache
    if _hw_symbols_cache is None:
        _hw_symbols_cache = {}
        if _HW_INC.is_file():
            for name, addr in _HW_SYMBOL_DEF_RE.findall(_HW_INC.read_text()):
                _hw_symbols_cache[name] = int(addr, 16)
    return _hw_symbols_cache


# Store-class opcodes: sta/stx/sty/stz plus the read-modify-write family
# (inc/dec/asl/lsr/rol/ror/trb/tsb), which also writes back to its operand.
_STORE_OPCODE_RE = re.compile(
    r"^\s*(?:@[A-Za-z0-9_]+:\s*)?"
    r"(?:sta|stx|sty|stz|inc|dec|asl|lsr|rol|ror|trb|tsb)(?:\.[bwl])?\s+"
    r"([\w$]+)",
    re.IGNORECASE | re.MULTILINE,
)


def bridge_asm_mmio_stores(func_name: str) -> set[int]:
    """MMIO addresses ($2100-$21FF, $4200-$43FF) that the asm body of
    `func_name` stores to — resolved via the hardware.inc symbol table
    (Pitfall 13/14) plus any literal hex operand already in that range.
    Read-only: used only to cross-check the CONTRACT's declared
    `mmio_effects`, never to decide translate-vs-delegate.
    """
    res = subprocess.run(
        [str(BRIDGE_BIN), "--root", str(UPSTREAM), "get-asm", func_name],
        capture_output=True, text=True,
    )
    if res.returncode != 0:
        return set()
    syms = hw_symbols()
    hits: set[int] = set()
    for m in _STORE_OPCODE_RE.finditer(res.stdout):
        operand = m.group(1)
        addr: Optional[int] = None
        if operand in syms:
            addr = syms[operand]
        elif operand.startswith("$"):
            try:
                addr = int(operand[1:], 16)
            except ValueError:
                addr = None
        if addr is not None and (0x2100 <= addr <= 0x21FF or 0x4200 <= addr <= 0x43FF):
            hits.add(addr)
    return hits


def bridge_get_address(module: str, func_name: str) -> Optional[int]:
    """Ground-truth address for `module::func_name` via ca65-bridge.

    Returns the full 24-bit SNES address (bank << 16 | offset_in_bank).
    """
    res = subprocess.run(
        [str(BRIDGE_BIN), "--root", str(UPSTREAM), "get-asm", func_name],
        capture_output=True, text=True,
    )
    if res.returncode != 0:
        return None
    # First line: "# address_hint: XXXX  instr=N  calls=N"
    first = res.stdout.splitlines()[0] if res.stdout else ""
    m = re.match(r"^# address_hint:\s*([0-9A-Fa-f]+)", first)
    if not m:
        return None
    offset = int(m.group(1), 16) & 0xFFFF
    bank = MODULE_BANK.get(module)
    if bank is None:
        return None
    return (bank << 16) | offset


def bridge_asm_has_indexed_store(func_name: str) -> bool:
    """True if the asm body of `func_name` contains an indexed store
    (sta/stx/sty/stz $XXXX,(x|y)). Such functions have a dynamic output
    address and cannot be tested by a fixed-slot parity harness.
    """
    res = subprocess.run(
        [str(BRIDGE_BIN), "--root", str(UPSTREAM), "get-asm", func_name],
        capture_output=True, text=True,
    )
    if res.returncode != 0:
        return False
    return bool(_INDEXED_STORE_RE.search(res.stdout))


# ---------------------------------------------------------------------------
# Contract parsing
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class Contract:
    func_name: str
    module: str
    addr24: int                # 24-bit SNES address, e.g. 0x03B74C
    inputs_reg: dict[str, Optional[int]]   # {'a': 8|16|None, 'x': 8|16|None, 'y': 8|16|None}
    inputs_ram: list[tuple[int, int]]      # [(addr, width), ...]
    output_ram: Optional[tuple[int, int]]  # (addr, width) or None
    entry_mf: bool
    entry_xf: bool
    entry_z: Optional[str]     # 'auto' or a C expression string
    entry_n: Optional[str]
    custom_spike: bool         # if True, skip generation
    compare_region: bool = False   # // SPIKE_COMPARE: region — memcmp the whole WRAM
                                   # (stack-masked) instead of a fixed output slot.
                                   # Validates routines with indexed stores (dynamic
                                   # output address), e.g. the OAM/sprite builders.
    mask_ranges: list = dataclasses.field(default_factory=list)
                                   # // SPIKE_MASK: lo-hi[, lo-hi...] — WRAM byte ranges
                                   # excluded from the region compare (dead DP scratch the
                                   # asm writes but the C port legitimately doesn't mirror).
    entry_dp: int = 0      # // CONTRACT: entry_mode: dp= — direct page at entry.
                            # Defaults to 0 (the historical hardcoded spike
                            # behavior) when the field is absent, for legacy
                            # contracts written before this was parsed.
    entry_db: int = 0x7E   # // CONTRACT: entry_mode: db= — data bank at entry.
                            # Defaults to 0x7E (the historical hardcoded spike
                            # behavior) when the field is absent.
    mmio_effects: list = dataclasses.field(default_factory=list)
                                   # // CONTRACT: mmio_effects: — declared hardware
                                   # register addresses the routine writes (Pitfall 13/16).
                                   # Empty list = "none" declared, or field absent (legacy
                                   # contract predating this schema — see the mismatch
                                   # check in main(), which only fires when the asm
                                   # actually stores to an address missing from this list).
    dma: Optional[str] = None     # // CONTRACT: dma: — 'none' | 'manual-loop' | 'delegate',
                                   # or None if the field is absent (legacy contract).
    output_regs: list = dataclasses.field(default_factory=list)
                                   # // SPIKE_OUTPUT_REG: a, c, z, n, v — compare CPU
                                   # register/flag state instead of a WRAM slot, for
                                   # routines with no WRAM footprint at all (their only
                                   # observable effect is the accumulator and/or NZVC —
                                   # e.g. BackAttackYOffset_s/l, $02:BB0B/BB1A). Without
                                   # this, a routine with no output_ram and no
                                   # SPIKE_COMPARE:region falls into the vacuous
                                   # "compare 0 to 0" fallback below, which always
                                   # "passes" without observing anything (found
                                   # 2026-07-05, see translator/runs/
                                   # D02BB0B_backattackyoffset_s_BLOCKED_vacuous_spike.txt).
    extra_src: list = dataclasses.field(default_factory=list)
                                   # // SPIKE_EXTRA_SRC: ../../ff4-gnw/battle/Add16.c, ...
                                   # additional ff4-gnw .c files (paths relative to
                                   # parity/) to link into this spike's Makefile rule,
                                   # for a translation that calls a SIBLING already-
                                   # dispatched routine (e.g. Mult16_c, Div16_c) by name
                                   # rather than through an auto-emittable "_emu"
                                   # delegation wrapper. Without this, such a call is an
                                   # unresolved symbol at link time (parity/Makefile's
                                   # $(CORE_SRC) is LakeSnes-only, never ff4-gnw sources)
                                   # -- this was the exact reason RandXA_c (which calls
                                   # Div16_c directly) sat at "spike does not compile
                                   # (inter-routine dependency)" for months (D038379).


_CONTRACT_RE = re.compile(r"//\s*CONTRACT:\s*\n((?:\s*//\s*[^\n]*\n)+)", re.MULTILINE)
_CUSTOM_RE = re.compile(r"//\s*CUSTOM_SPIKE:\s*yes", re.IGNORECASE)
_REGION_RE = re.compile(r"//\s*SPIKE_COMPARE:\s*region", re.IGNORECASE)
_MASK_RE = re.compile(r"//\s*SPIKE_MASK:\s*([0-9A-Fa-fx,\s\-]+)", re.IGNORECASE)
_OUTPUT_REG_RE = re.compile(r"//\s*SPIKE_OUTPUT_REG:\s*([A-Za-z,\s]+)", re.IGNORECASE)
_VALID_OUTPUT_REGS = ("a", "x", "y", "c", "z", "v", "n")
_EXTRA_SRC_RE = re.compile(r"//\s*SPIKE_EXTRA_SRC:\s*([^\n]+)", re.IGNORECASE)


def parse_extra_src(text: str) -> list:
    """Parse `// SPIKE_EXTRA_SRC: path1, path2, ...` into a deduped, ordered
    list of extra source files (paths as written, relative to parity/) to
    link into this spike's Makefile rule alongside $(CORE_SRC)."""
    m = _EXTRA_SRC_RE.search(text)
    if not m:
        return []
    out = []
    for part in m.group(1).split(","):
        part = part.strip()
        if part and part not in out:
            out.append(part)
    return out


def parse_output_regs(text: str) -> list:
    """Parse `// SPIKE_OUTPUT_REG: a, c, z, n, v` into an ordered, deduped list
    of Cpu field names to compare after the run — the register/flag-output
    mode, an alternative to `output_ram`/`SPIKE_COMPARE: region` for routines
    whose only observable effect is the accumulator and/or NZVC flags."""
    m = _OUTPUT_REG_RE.search(text)
    if not m:
        return []
    out = []
    for part in m.group(1).split(","):
        part = part.strip().lower()
        if not part:
            continue
        if part in _VALID_OUTPUT_REGS:
            if part not in out:
                out.append(part)
        else:
            sys.stderr.write(f"[gen] skipping unknown SPIKE_OUTPUT_REG entry: {part!r}\n")
    return out


def parse_mask_ranges(text: str) -> list:
    """Parse `// SPIKE_MASK: lo-hi[, lo-hi...]` into [(lo, hi), ...] (inclusive).

    Ranges name WRAM byte addresses excluded from the region compare — dead DP
    scratch the asm writes (e.g. `stx $1c`) but the C port doesn't mirror.
    """
    m = _MASK_RE.search(text)
    if not m:
        return []
    ranges = []
    for part in m.group(1).split(","):
        part = part.strip()
        if not part or "-" not in part:
            continue
        lo_s, hi_s = part.split("-", 1)
        try:
            ranges.append((int(lo_s, 16), int(hi_s, 16)))
        except ValueError:
            continue
    return ranges
_REVERSED_RE = re.compile(
    r"REVERSED_FUNCTION:\s*(\w+)::(\w+)\s*\(\$([0-9A-Fa-f]+):([0-9A-Fa-f]+)\)"
)
_DELEGATED_RE = re.compile(
    r"DELEGATED_FUNCTION:\s*(\w+)::(\w+)\s*\(\$([0-9A-Fa-f]+):([0-9A-Fa-f]+)\)"
)


def parse_contract(text: str, source_path: Optional[Path] = None) -> Optional[Contract]:
    """Parse the C source's CONTRACT block.

    Returns None if missing or the file is a delegated wrapper.

    The 24-bit address is resolved via ca65-bridge from
    `port/<module>/<func_name>.c` (the file path is the ground truth for
    module and function name). The LLM-emitted REVERSED_FUNCTION value is
    cross-checked: if it disagrees with the bridge, we warn and trust the
    bridge. This prevents the bank/offset confusion observed in the first
    real run (see ff4-port::phase-4-4-first-real-run).
    """
    if _DELEGATED_RE.search(text):
        # Delegated wrapper — no asm-vs-C parity to test, smoke is enough.
        return None

    # Ground truth: module + name from the file path (preferred over the LLM line).
    module: Optional[str] = None
    name: Optional[str] = None
    if source_path is not None:
        # Standard layout: port/<module>/<name>.c
        if source_path.parent.parent.name == "port":
            module = source_path.parent.name
            name = source_path.stem
        # Bench / parallel-run layout: */<module>/<name>.c where <module>
        # is one of the known module names.
        elif source_path.parent.name in MODULE_BANK:
            module = source_path.parent.name
            name = source_path.stem

    m_rev = _REVERSED_RE.search(text)
    if m_rev:
        rev_module, rev_name = m_rev.group(1), m_rev.group(2)
        rev_bank, rev_offset = int(m_rev.group(3), 16), int(m_rev.group(4), 16)
        rev_addr = (rev_bank << 16) | rev_offset
        # Use the LLM module/name only if the file path didn't disambiguate.
        module = module or rev_module
        name = name or rev_name
    else:
        rev_addr = None

    if module is None or name is None:
        sys.stderr.write("[gen] could not determine module/function from file path "
                         "and no REVERSED_FUNCTION line found.\n")
        return None

    true_addr = bridge_get_address(module, name)
    if true_addr is None:
        # The port name may differ from the asm label (e.g. DrawMonsterSprite_c
        # ports UpdateCharPalette @da73). Fall back to the REVERSED_FUNCTION
        # address when the name doesn't resolve.
        if rev_addr is None:
            sys.stderr.write(f"[gen] ca65-bridge could not resolve {module}::{name}\n")
            return None
        sys.stderr.write(
            f"[gen] ca65-bridge could not resolve {module}::{name}; using "
            f"REVERSED_FUNCTION ${rev_addr >> 16:02X}:{rev_addr & 0xFFFF:04X} "
            "(port name differs from asm label).\n"
        )
        addr24 = rev_addr
    else:
        if rev_addr is not None and rev_addr != true_addr:
            sys.stderr.write(
                f"[gen] WARNING: REVERSED_FUNCTION says ${rev_addr >> 16:02X}:"
                f"{rev_addr & 0xFFFF:04X} but ca65-bridge says "
                f"${true_addr >> 16:02X}:{true_addr & 0xFFFF:04X}. "
                f"Trusting the bridge.\n"
            )
        addr24 = true_addr

    m_block = _CONTRACT_RE.search(text)
    if not m_block:
        return None
    block = m_block.group(1)
    block_lines = [ln.strip().lstrip("/").strip() for ln in block.splitlines()]

    def find(key: str) -> Optional[str]:
        for ln in block_lines:
            if ln.startswith(key):
                return ln[len(key):].strip().lstrip(":").strip()
        return None

    custom_spike = bool(_CUSTOM_RE.search(text))

    def parse_reg(spec: str) -> dict[str, Optional[int]]:
        # "a=8, x=16, y=none"
        out: dict[str, Optional[int]] = {"a": None, "x": None, "y": None}
        for part in spec.split(","):
            part = part.strip()
            if "=" not in part:
                continue
            k, v = [s.strip() for s in part.split("=", 1)]
            # Strip a trailing semantic annotation, e.g. "1(upper_bound)" → "1".
            v = re.sub(r"\(.*$", "", v).strip()
            if k in out:
                if v.lower() == "none" or not v:
                    out[k] = None
                else:
                    try:
                        out[k] = int(v)
                    except ValueError:
                        sys.stderr.write(f"[gen] skipping malformed reg spec part: {part!r}\n")
                        out[k] = None
        return out

    def parse_ram(spec: str) -> list[tuple[int, int]]:
        # "0x38FA=1, 0x38FB=1"
        # Strip any trailing `#` comment (LLMs sometimes add inline notes).
        spec = re.sub(r"#.*$", "", spec).strip()
        out: list[tuple[int, int]] = []
        if not spec or spec.lower() == "none":
            return out
        for part in spec.split(","):
            part = part.strip()
            if "=" not in part:
                continue
            addr_s, width_s = [s.strip() for s in part.split("=", 1)]
            # Also strip any inline `#` per-value comment, in case the LLM
            # put one mid-spec rather than at the end, plus a trailing
            # semantic annotation, e.g. "1(incremented)" → "1".
            width_s = re.sub(r"#.*$", "", width_s).strip()
            width_s = re.sub(r"\(.*$", "", width_s).strip()
            # A range address ("0x1900..19FF") has no single width to fuzz here.
            addr_s = addr_s.split("..", 1)[0].strip()
            try:
                out.append((int(addr_s, 16), int(width_s)))
            except ValueError:
                sys.stderr.write(f"[gen] skipping malformed RAM spec part: {part!r}\n")
        return out

    def parse_single_ram(spec: str) -> Optional[tuple[int, int]]:
        ram = parse_ram(spec)
        return ram[0] if ram else None

    def parse_mmio(spec: str) -> list[int]:
        # "none" | "$2100, $420B, ..."
        spec = re.sub(r"#.*$", "", spec).strip()
        out: list[int] = []
        if not spec or spec.lower() == "none":
            return out
        for part in spec.split(","):
            part = part.strip().lstrip("$")
            if not part:
                continue
            try:
                out.append(int(part, 16))
            except ValueError:
                sys.stderr.write(f"[gen] skipping malformed mmio_effects part: {part!r}\n")
        return out

    def parse_mode(spec: str, key: str) -> bool:
        # "mf=true, xf=false, dp=0x0, db=0x7E"
        for part in spec.split(","):
            part = part.strip()
            if part.lower().startswith(key + "="):
                return part.split("=", 1)[1].strip().lower() == "true"
        return True  # default A 8-bit

    def parse_hex_mode(spec: str, key: str, default: int) -> int:
        # "mf=true, xf=false, dp=0x0600, db=0x00" -> dp=0x0600. Falls back to
        # `default` (the historical hardcoded spike value) if the key is
        # absent, so legacy contracts predating dp/db parsing keep working.
        for part in spec.split(","):
            part = part.strip()
            if part.lower().startswith(key + "="):
                val = part.split("=", 1)[1].strip()
                val = re.sub(r"\(.*$", "", val).strip()
                try:
                    return int(val, 16) if val.lower().startswith("0x") else int(val)
                except ValueError:
                    sys.stderr.write(f"[gen] malformed entry_mode {key}={val!r}, "
                                      f"using default 0x{default:X}\n")
                    return default
        return default

    def parse_flag(spec: str, key: str) -> Optional[str]:
        for part in spec.split(","):
            part = part.strip()
            if part.lower().startswith(key + "="):
                return part.split("=", 1)[1].strip()
        return None

    inputs_reg = parse_reg(find("inputs_reg") or "")
    inputs_ram = parse_ram(find("inputs_ram") or "")
    output_ram = parse_single_ram(find("output_ram") or "")
    entry_mode = find("entry_mode") or ""
    entry_mf = parse_mode(entry_mode, "mf")
    entry_xf = parse_mode(entry_mode, "xf")
    entry_dp = parse_hex_mode(entry_mode, "dp", 0)
    entry_db = parse_hex_mode(entry_mode, "db", 0x7E)
    entry_flags = find("entry_flags") or ""
    entry_z = parse_flag(entry_flags, "z")
    entry_n = parse_flag(entry_flags, "n")
    mmio_effects = parse_mmio(find("mmio_effects") or "")
    dma_raw = find("dma")
    dma = dma_raw.strip().lower() if dma_raw else None

    return Contract(
        func_name=name,
        module=module,
        addr24=addr24,
        inputs_reg=inputs_reg,
        inputs_ram=inputs_ram,
        output_ram=output_ram,
        entry_mf=entry_mf,
        entry_xf=entry_xf,
        entry_dp=entry_dp,
        entry_db=entry_db,
        entry_z=entry_z,
        entry_n=entry_n,
        custom_spike=custom_spike,
        compare_region=bool(_REGION_RE.search(text)),
        mask_ranges=parse_mask_ranges(text),
        mmio_effects=mmio_effects,
        dma=dma,
        output_regs=parse_output_regs(text),
        extra_src=parse_extra_src(text),
    )


# ---------------------------------------------------------------------------
# Helper auto-emission
# ---------------------------------------------------------------------------

# An identifier ending in `_emu` called as `name_emu(snes, ...)`.
_EMU_CALL_RE = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)_emu\s*\(")

# Helpers that are pre-defined inside the spike skeleton itself (NOT something
# we need to auto-emit, even if the translation references them).
_SKELETON_BUILTINS = set()  # currently empty; the skeleton defines no *_emu

# Translation between asm function names and the *_emu helper name we emit.
# Names from the CONTRACT block use the C identifier convention, so the
# corresponding ca65 label has the same characters minus the `_emu` suffix
# AND with the first letter uppercased (mult8_emu → Mult8). We try both forms.
def _candidate_asm_names(helper_stub: str) -> list[str]:
    """Possible ca65 labels for a `<stub>_emu` helper."""
    # mult8 → Mult8, rand99 → Rand99, mult16 → Mult16, apply_dmg_mult → ApplyDmgMult
    parts = helper_stub.split("_")
    pascal = "".join(p[:1].upper() + p[1:] for p in parts if p)
    return [pascal, helper_stub, helper_stub.upper(), helper_stub.lower()]


def collect_required_helpers(c_body: str) -> list[str]:
    """Return the list of `<name>_emu` helper stubs used by the translation."""
    seen: dict[str, None] = {}
    for m in _EMU_CALL_RE.finditer(c_body):
        stub = m.group(1)
        if stub in _SKELETON_BUILTINS:
            continue
        # If the translation already defines the helper itself (e.g. a static
        # `<name>_emu` body inside port/<module>/<name>.c), skip.
        defn_re = re.compile(
            rf"(?:^|\n)\s*static\s+\S+\s+{re.escape(stub)}_emu\s*\(", re.MULTILINE
        )
        if defn_re.search(c_body):
            continue
        seen[stub] = None
    return list(seen.keys())


def emit_helper(stub: str, module: str) -> Optional[str]:
    """Resolve a stub's asm address and emit a trivial delegation wrapper."""
    for cand in _candidate_asm_names(stub):
        addr = bridge_get_address(module, cand)
        if addr is not None:
            return _EMU_HELPER_TEMPLATE.format(stub=stub, asm_name=cand, addr24=addr)
    return None


_EMU_HELPER_TEMPLATE = """\
// Auto-emitted delegation wrapper for {asm_name} @ ${addr24:06X} (referenced as {stub}_emu)
static uint16_t {stub}_emu(Snes *snes) {{
    Cpu *c = snes->cpu;
    uint16_t saved_a=c->a, saved_x=c->x, saved_y=c->y, saved_sp=c->sp;
    uint16_t saved_pc=c->pc, saved_dp=c->dp;
    uint8_t saved_k=c->k, saved_db=c->db;
    bool saved_mf=c->mf, saved_xf=c->xf;
    c->dp = 0; c->db = 0x7E;
    c->mf = true; c->xf = false;
    run_emulated_func(snes, 0x{addr24:06X}u);
    uint16_t result = c->a;
    c->x=saved_x; c->y=saved_y;
    c->sp=saved_sp; c->pc=saved_pc; c->dp=saved_dp;
    c->k=saved_k; c->db=saved_db;
    c->mf=saved_mf; c->xf=saved_xf;
    c->a = saved_a;
    return result;
}}
"""


# ---------------------------------------------------------------------------
# Spike generation
# ---------------------------------------------------------------------------

SPIKE_SKELETON = r"""// AUTO-GENERATED parity spike for {module}::{func_name} @ ${bank:02X}:{offset:04X}.
// Generated by translator/generate_spike.py from port/{module}/{func_name}.c.
// DO NOT EDIT BY HAND — regenerate via:
//   python translator/generate_spike.py port/{module}/{func_name}.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes.h"
#include "cpu.h"

#define TARGET_ADDR_24 0x{addr24:06X}u

// Forward declarations for helpers defined later in this TU but used by the
// inlined LLM translation below.
static void run_emulated_func(Snes *snes, uint32_t pc24);
// External linkage (not "static inline"): a SPIKE_EXTRA_SRC file (e.g. an
// already-dispatched sibling ff4-gnw/*.c called directly, not via an
// auto-emitted _emu wrapper) compiles as its own translation unit and needs
// a real symbol to link against -- ff4-gnw's own read16/write16 (in
// ff4_helpers.c) are implicitly declared (int) by such callers, same as on
// the device. Only one spike .c ever defines these per binary, so external
// linkage can't collide.
uint16_t read16(const uint8_t *ram, int addr);
void write16(uint8_t *ram, int addr, uint16_t v);

// ---------------------------------------------------------------------------
// Auto-emitted *_emu delegation helpers (one per stub referenced by the
// translation but not defined in port/{module}/{func_name}.c).
// ---------------------------------------------------------------------------

{auto_emu_helpers}

// ---------------------------------------------------------------------------
// LLM-translated C function (verbatim copy from port/{module}/{func_name}.c)
// ---------------------------------------------------------------------------

{c_translation}

// ---------------------------------------------------------------------------
// Common helpers (same shape as the manual M2/M3/M4 spikes)
// ---------------------------------------------------------------------------

static uint8_t *read_file(const char *path, size_t *out_len) {{
    FILE *f = fopen(path, "rb");
    if (!f) {{ perror(path); return NULL; }}
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *buf = malloc(sz);
    if (fread(buf, 1, sz, f) != (size_t)sz) {{ free(buf); fclose(f); return NULL; }}
    fclose(f); *out_len = sz; return buf;
}}

static inline uint32_t stack_addr(const Cpu *cpu) {{
    return cpu->e ? (0x0100u | (cpu->sp & 0xFFu)) : cpu->sp;
}}

static void run_emulated_func(Snes *snes, uint32_t pc24) {{
    Cpu *cpu = snes->cpu;
    uint16_t sp_save = cpu->sp;
    snes->ram[stack_addr(cpu)] = 0x12; cpu->sp--;
    snes->ram[stack_addr(cpu)] = 0x34; cpu->sp--;
    cpu->k = (uint8_t)(pc24 >> 16);
    cpu->pc = (uint16_t)(pc24 & 0xFFFF);
    long max_ops = 200000;
    while (cpu->sp != sp_save && max_ops-- > 0) {{
        cpu_runOpcode(cpu);
        if (cpu->waiting || cpu->stopped) break;
    }}
}}

typedef struct {{
    uint8_t ram[0x20000];
    uint16_t a, x, y, sp, pc, dp;
    uint8_t k, db;
    bool c, z, v, n, i, d, xf, mf, e;
    bool waiting, stopped, irqWanted, nmiWanted;
}} Snap;

static void snap_take(Snap *s, Snes *snes) {{
    memcpy(s->ram, snes->ram, sizeof(s->ram));
    Cpu *c = snes->cpu;
    s->a=c->a; s->x=c->x; s->y=c->y; s->sp=c->sp; s->pc=c->pc; s->dp=c->dp;
    s->k=c->k; s->db=c->db;
    s->c=c->c; s->z=c->z; s->v=c->v; s->n=c->n;
    s->i=c->i; s->d=c->d; s->xf=c->xf; s->mf=c->mf; s->e=c->e;
    s->waiting=c->waiting; s->stopped=c->stopped;
    s->irqWanted=c->irqWanted; s->nmiWanted=c->nmiWanted;
}}
static void snap_restore(const Snap *s, Snes *snes) {{
    memcpy(snes->ram, s->ram, sizeof(s->ram));
    Cpu *c = snes->cpu;
    c->a=s->a; c->x=s->x; c->y=s->y; c->sp=s->sp; c->pc=s->pc; c->dp=s->dp;
    c->k=s->k; c->db=s->db;
    c->c=s->c; c->z=s->z; c->v=s->v; c->n=s->n;
    c->i=s->i; c->d=s->d; c->xf=s->xf; c->mf=s->mf; c->e=s->e;
    c->waiting=s->waiting; c->stopped=s->stopped;
    c->irqWanted=s->irqWanted; c->nmiWanted=s->nmiWanted;
}}

uint16_t read16(const uint8_t *ram, int addr) {{
    return (uint16_t)(ram[addr] | (ram[addr + 1] << 8));
}}
void write16(uint8_t *ram, int addr, uint16_t v) {{
    ram[addr] = v & 0xFF; ram[addr + 1] = (v >> 8) & 0xFF;
}}

// ---------------------------------------------------------------------------
// asm wrapper from the contract
// ---------------------------------------------------------------------------

static void run_asm(Snes *snes{asm_args_decl}) {{
    Cpu *c = snes->cpu;
    c->dp = {entry_dp};
    c->db = {entry_db};
    c->mf = {entry_mf};
    c->xf = {entry_xf};
    c->a = 0; c->x = 0; c->y = 0;
{asm_reg_setup}
{asm_flags_setup}
    run_emulated_func(snes, TARGET_ADDR_24);
}}

// ---------------------------------------------------------------------------
// Host PRNG
// ---------------------------------------------------------------------------

static uint32_t host_rng_state = 0xC0FFEEu;
static uint32_t host_rng(void) {{
    uint32_t x = host_rng_state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    host_rng_state = x; return x;
}}

// ---------------------------------------------------------------------------
// Trial driver
// ---------------------------------------------------------------------------

int main(int argc, char **argv) {{
    if (argc < 2) {{
        fprintf(stderr, "usage: %s <rom.sfc> [n_trials]\n", argv[0]);
        return 1;
    }}
    int n_trials = (argc >= 3) ? atoi(argv[2]) : 1000;

    size_t rom_len = 0;
    uint8_t *rom = read_file(argv[1], &rom_len);
    if (!rom) return 2;

    Snes *snes = snes_init();
    if (!snes_loadRom(snes, rom, rom_len)) {{ fprintf(stderr, "loadRom fail\n"); return 3; }}
    snes_reset(snes, true);
    for (int i = 0; i < 60; i++) snes_runFrame(snes);
    snes->cpu->i = true; snes->cpu->nmiWanted = false; snes->cpu->irqWanted = false;

    Snap baseline;
    snap_take(&baseline, snes);

    fprintf(stderr, "{func_name} asm vs C : %d trials\n", n_trials);

    int fails = 0;
    for (int trial = 0; trial < n_trials; trial++) {{
        snap_restore(&baseline, snes);

        // Randomise inputs.
{ram_input_setup}
{reg_input_random}

        // Pre-call snapshot for replaying C with the exact same state.
        Snap pre;
        snap_take(&pre, snes);

        // Run ASM
        run_asm(snes{asm_args_pass});
{capture_asm_output}

        // Restore and run C
        snap_restore(&pre, snes);
{c_call}
{capture_c_output}

{compare_block}
    }}

    printf("\n=== summary === trials: %d, fails: %d\n", n_trials, fails);
    free(rom);
    return fails == 0 ? 0 : 1;
}}
"""


def _strip_trailers(c_translation: str) -> str:
    """Remove the trailing metadata block (// PITFALLS:, // HELPERS:,
    // CONTRACT:, REVERSED_FUNCTION:, DELEGATED_FUNCTION:) before inlining.

    LLMs sometimes emit `REVERSED_FUNCTION:` without a leading `//`, which
    breaks the C compile when the translation is inlined verbatim. We cut
    at the first trailer marker we recognise.
    """
    lines = c_translation.splitlines()
    cut_at = len(lines)
    trailer_pat = re.compile(
        r"^\s*(?://\s*)?(PITFALLS:|HELPERS:|CONTRACT:|REVERSED_FUNCTION:|DELEGATED_FUNCTION:)",
        re.IGNORECASE,
    )
    for i, line in enumerate(lines):
        if trailer_pat.match(line):
            cut_at = i
            break
    return "\n".join(lines[:cut_at]).rstrip() + "\n"


def render_spike(c_translation: str, contract: Contract) -> str:
    bank = (contract.addr24 >> 16) & 0xFF
    offset = contract.addr24 & 0xFFFF

    # Strip the metadata trailers (they would break the C compile).
    c_translation = _strip_trailers(c_translation)

    # Auto-emit *_emu helpers referenced by the translation but not defined.
    required = collect_required_helpers(c_translation)
    helpers: list[str] = []
    for stub in required:
        h = emit_helper(stub, contract.module)
        if h is None:
            sys.stderr.write(
                f"[gen] WARNING: could not resolve `{stub}_emu` to an asm label "
                f"via ca65-bridge — leaving it unresolved (compile will fail).\n"
            )
        else:
            helpers.append(h)
    auto_emu_helpers = "\n".join(helpers) if helpers else "// (no auto-emitted helpers)"

    # Build asm args declaration / passing.
    asm_args_decl = ""
    asm_args_pass = ""
    asm_reg_setup_lines = []
    reg_input_random_lines = []
    c_args = []

    if contract.inputs_reg.get("a") is not None:
        ty = "uint16_t" if contract.inputs_reg["a"] == 16 else "uint8_t"
        asm_args_decl += f", {ty} arg_a"
        asm_args_pass += ", arg_a"
        asm_reg_setup_lines.append(f"    c->a = arg_a;")
        reg_input_random_lines.append(f"        {ty} arg_a = ({ty})host_rng();")
        c_args.append("arg_a")
    if contract.inputs_reg.get("x") is not None:
        ty = "uint16_t" if contract.inputs_reg["x"] == 16 else "uint8_t"
        asm_args_decl += f", {ty} arg_x"
        asm_args_pass += ", arg_x"
        asm_reg_setup_lines.append(f"    c->x = arg_x;")
        reg_input_random_lines.append(f"        {ty} arg_x = ({ty})host_rng();")
        c_args.append("arg_x")
    if contract.inputs_reg.get("y") is not None:
        ty = "uint16_t" if contract.inputs_reg["y"] == 16 else "uint8_t"
        asm_args_decl += f", {ty} arg_y"
        asm_args_pass += ", arg_y"
        asm_reg_setup_lines.append(f"    c->y = arg_y;")
        reg_input_random_lines.append(f"        {ty} arg_y = ({ty})host_rng();")
        c_args.append("arg_y")

    # Flags setup
    asm_flags_lines = []
    if contract.entry_z and contract.entry_z.lower() != "auto":
        asm_flags_lines.append(f"    c->z = ({contract.entry_z});")
    elif contract.inputs_reg.get("a") is not None:
        asm_flags_lines.append("    c->z = (arg_a == 0);")
    if contract.entry_n and contract.entry_n.lower() != "auto":
        asm_flags_lines.append(f"    c->n = ({contract.entry_n});")
    elif contract.inputs_reg.get("a") is not None:
        bits = contract.inputs_reg["a"]
        mask = "0x80" if bits == 8 else "0x8000"
        asm_flags_lines.append(f"    c->n = ((arg_a & {mask}) != 0);")

    # RAM inputs randomisation
    ram_input_setup_lines = []
    for addr, width in contract.inputs_ram:
        if width == 1:
            ram_input_setup_lines.append(
                f"        snes->ram[0x{addr:04X}] = (uint8_t)host_rng();"
            )
        else:
            ram_input_setup_lines.append(
                f"        write16(snes->ram, 0x{addr:04X}, (uint16_t)host_rng());"
            )

    # Slot-comparison fragment (default): compare a single contract output slot.
    _slot_compare = (
        "        bool ok = (out_asm == out_c);\n"
        "        if (!ok) {\n"
        "            printf(\"trial %4d : asm=%u c=%u  FAIL\\n\", trial, out_asm, out_c);\n"
        "            fails++;\n"
        "        }"
    )

    # Output capture + comparison
    if contract.output_regs:
        # Register/flag-output mode: compare snes->cpu->{a,x,y,c,z,v,n} directly
        # for routines with no WRAM footprint at all -- e.g. BackAttackYOffset_s/l
        # ($02:BB0B/BB1A), whose only observable effect is the accumulator and
        # NZVC. Both sides mutate the SAME Cpu struct in place (run_asm via the
        # interpreter, the C body directly), so this only needs to read the
        # declared fields at the two existing capture points, same as the
        # output_ram slot mode below -- no new plumbing into run_asm/c_call.
        _reg_c_type = {"a": "uint16_t", "x": "uint16_t", "y": "uint16_t",
                       "c": "bool", "z": "bool", "v": "bool", "n": "bool"}
        asm_lines, c_lines, cmp_terms, fmt_parts, fmt_args = [], [], [], [], []
        for reg in contract.output_regs:
            ty = _reg_c_type[reg]
            asm_lines.append(f"        {ty} out_asm_{reg} = snes->cpu->{reg};")
            c_lines.append(f"        {ty} out_c_{reg} = snes->cpu->{reg};")
            cmp_terms.append(f"(out_asm_{reg} == out_c_{reg})")
            fmt_parts.append(f"{reg}=asm:%d/c:%d")
            fmt_args.append(f"out_asm_{reg}, out_c_{reg}")
        capture_asm_output = "\n".join(asm_lines)
        capture_c_output = "\n".join(c_lines)
        compare_block = (
            f"        bool ok = {' && '.join(cmp_terms)};\n"
            "        if (!ok) {\n"
            f'            printf("trial %4d : {" ".join(fmt_parts)}  FAIL\\n", trial, '
            + ", ".join(fmt_args) + ");\n"
            "            fails++;\n"
            "        }"
        )
    elif contract.compare_region:
        # Region mode: snapshot the full WRAM after each run, then memcmp,
        # masking the stack page ($0100-$01FF) where the asm's push/pull frames
        # and run_emulated_func's return frame live (the inlined C never touches
        # the stack). Handles indexed stores: we observe wherever the write
        # landed instead of reading a fixed (and necessarily wrong) slot.
        capture_asm_output = "        Snap asm_post; snap_take(&asm_post, snes);"
        capture_c_output = "        Snap c_post; snap_take(&c_post, snes);"
        _mask_lines = "".join(
            f"            if (a >= 0x{lo:05X} && a <= 0x{hi:05X}) continue;  /* SPIKE_MASK scratch */\n"
            for (lo, hi) in contract.mask_ranges
        )
        compare_block = (
            "        /* Mask the stack page relative to SP: the asm's push/pull and\n"
            "         * run_emulated_func's return frame mutate it, the inlined C never\n"
            "         * touches the stack. FF4's combat stack lives in $02xx (SP~$02E5),\n"
            "         * the field stack in $01xx — so derive the page from pre.sp. */\n"
            "        int sp_page = pre.sp & 0xFF00;\n"
            "        int diff = -1;\n"
            "        for (int a = 0; a < 0x20000; a++) {\n"
            "            if (a >= sp_page && a < sp_page + 0x100) continue;  /* mask stack page */\n"
            + _mask_lines +
            "            if (asm_post.ram[a] != c_post.ram[a]) { diff = a; break; }\n"
            "        }\n"
            "        bool ok = (diff < 0);\n"
            "        if (!ok) {\n"
            "            printf(\"trial %4d : first WRAM diff at $%05X  asm=%02X c=%02X  FAIL\\n\",\n"
            "                   trial, diff, asm_post.ram[diff], c_post.ram[diff]);\n"
            "            fails++;\n"
            "        }"
        )
    elif contract.output_ram is None:
        # No single-output contract; fall back to "no comparison" — caller must
        # add a CUSTOM_SPIKE marker. We still generate the wrapper for runs.
        capture_asm_output = "        uint16_t out_asm = 0; (void)snes;"
        capture_c_output = "        uint16_t out_c = 0;"
        compare_block = _slot_compare
    else:
        addr, width = contract.output_ram
        if width == 1:
            capture_asm_output = f"        uint8_t out_asm = snes->ram[0x{addr:04X}];"
            capture_c_output = f"        uint8_t out_c = snes->ram[0x{addr:04X}];"
        else:
            capture_asm_output = f"        uint16_t out_asm = read16(snes->ram, 0x{addr:04X});"
            capture_c_output = f"        uint16_t out_c = read16(snes->ram, 0x{addr:04X});"
        compare_block = _slot_compare

    # C call: FF4 ported bodies are `void Name_c(Snes*)` — they read cpu->{a,x,y}
    # and cpu->{mf,xf,db,dp} directly, they do NOT take the reg inputs as C args.
    # So mirror run_asm's entry setup ON `snes` (snap_restore(&pre) reset the regs
    # to baseline) and call with just (snes), instead of passing arg_* positionally.
    c_entry_lines = [
        f"        snes->cpu->dp = 0x{contract.entry_dp:04X};",
        f"        snes->cpu->db = 0x{contract.entry_db:02X};",
        f"        snes->cpu->mf = {'true' if contract.entry_mf else 'false'};",
        f"        snes->cpu->xf = {'true' if contract.entry_xf else 'false'};",
        "        snes->cpu->a = 0; snes->cpu->x = 0; snes->cpu->y = 0;",
    ]
    if contract.inputs_reg.get("a") is not None:
        c_entry_lines.append("        snes->cpu->a = arg_a;")
    if contract.inputs_reg.get("x") is not None:
        c_entry_lines.append("        snes->cpu->x = arg_x;")
    if contract.inputs_reg.get("y") is not None:
        c_entry_lines.append("        snes->cpu->y = arg_y;")
    c_call = "\n".join(c_entry_lines) + "\n" + f"        {contract.func_name}_c(snes);"

    return SPIKE_SKELETON.format(
        module=contract.module,
        func_name=contract.func_name,
        bank=bank,
        offset=offset,
        addr24=contract.addr24,
        c_translation=c_translation,
        auto_emu_helpers=auto_emu_helpers,
        asm_args_decl=asm_args_decl,
        asm_args_pass=asm_args_pass,
        asm_reg_setup="\n".join(asm_reg_setup_lines) or "    // no register inputs",
        asm_flags_setup="\n".join(asm_flags_lines) or "    // no entry flags",
        entry_mf="true" if contract.entry_mf else "false",
        entry_xf="true" if contract.entry_xf else "false",
        entry_dp=f"0x{contract.entry_dp:04X}",
        entry_db=f"0x{contract.entry_db:02X}",
        ram_input_setup="\n".join(ram_input_setup_lines) or "        // no RAM inputs",
        reg_input_random="\n".join(reg_input_random_lines),
        c_call=c_call,
        capture_asm_output=capture_asm_output,
        capture_c_output=capture_c_output,
        compare_block=compare_block,
    )


# ---------------------------------------------------------------------------
# Makefile patching
# ---------------------------------------------------------------------------

_MK_RULE = """
# AUTO: spike for {func_name}
BIN_AUTO_{upper} := ff4-spike-auto-{slug}
{auto_bin}: src/spike_{slug}_auto.c $(CORE_SRC)
\t$(CC) $(CFLAGS) -o $@ src/spike_{slug}_auto.c $(CORE_SRC) $(LDFLAGS)
"""


def add_makefile_rule(name: str, suffix: str = "auto", extra_src: list = None) -> str:
    """Append a build rule for the auto spike. Returns the binary name.

    extra_src: additional source files (e.g. sibling ff4-gnw .c files a
    translation calls directly, like Mult16_c/Div16_c) to link alongside
    $(CORE_SRC) -- see Contract.extra_src / SPIKE_EXTRA_SRC."""
    slug = name.lower()
    bin_name = f"ff4-spike-{suffix}-{slug}"
    text = PARITY_MAKEFILE.read_text()
    if bin_name in text:
        return bin_name
    extra = " " + " ".join(extra_src) if extra_src else ""
    rule = f"""
# AUTO ({suffix}): spike for {name}
{bin_name}: src/spike_{slug}_{suffix}.c $(CORE_SRC){extra}
\t$(CC) $(CFLAGS) -o $@ src/spike_{slug}_{suffix}.c $(CORE_SRC){extra} $(LDFLAGS)
"""
    PARITY_MAKEFILE.write_text(text + rule)
    return bin_name


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("source", type=Path, help="port/<module>/<name>.c file")
    ap.add_argument("--build", action="store_true", help="run `make <spike>` after generation")
    ap.add_argument("--run", type=int, default=0, help="run N trials after building (requires --build)")
    ap.add_argument("--rom", type=Path, default=ROOT / "upstream/rom/ff4-jp1.sfc")
    ap.add_argument("--spike-suffix", default="auto",
                    help="Suffix used for the generated spike file "
                         "(parity/src/spike_<name>_<suffix>.c). Set per "
                         "model when running parallel benches.")
    ap.add_argument("--run-timeout", type=int, default=20,
                    help="Hard timeout in seconds on the spike binary "
                         "execution. A binary that does not finish N trials "
                         "in this budget is killed (likely infinite loop).")
    args = ap.parse_args(argv)

    text = args.source.read_text()
    contract = parse_contract(text, source_path=args.source.resolve())
    if contract is None:
        if _DELEGATED_RE.search(text):
            sys.stderr.write(f"[gen] {args.source} is a delegate wrapper — no spike to generate.\n")
            return 0
        if _CUSTOM_RE.search(text):
            sys.stderr.write(f"[gen] {args.source} marked CUSTOM_SPIKE: yes — skipping auto-gen.\n")
            return 0
        sys.stderr.write(f"[gen] could not parse a CONTRACT block in {args.source}.\n"
                          "       Add // CONTRACT: ... per prompts/reverser_system.md.\n")
        return 1

    if contract.custom_spike:
        sys.stderr.write(f"[gen] {args.source} marked CUSTOM_SPIKE: yes — skipping auto-gen.\n")
        return 0

    # Safety net (Pitfall 13/16): the asm stores to a hardware register the
    # CONTRACT does not declare in mmio_effects. The spike only ever compares
    # output_ram (plain WRAM) — it is structurally blind to bus/VRAM/OAM/CGRAM
    # side effects, so an undeclared MMIO store yields a FALSE L2 (the exact
    # historical bug: InitMapRAM declared only output_ram 0x06FB and silently
    # also hit $2100/$420C/$4200; TfrBGGfx listed the $43xx DMA registers as
    # WRAM). This is a CONTRACT correctness bug, not a harness limitation —
    # unlike CUSTOM_SPIKE below, it is not silently skipped (exit 0): it hard
    # fails (exit 3) so it surfaces distinctly instead of being fixed by
    # re-declaring `mmio_effects` and re-running. A legacy contract with no
    # mmio_effects field at all is treated the same as an empty declaration:
    # this only fires when the asm demonstrably writes an undeclared address.
    undeclared_mmio = bridge_asm_mmio_stores(contract.func_name) - set(contract.mmio_effects)
    if undeclared_mmio:
        addrs = ", ".join(f"${a:04X}" for a in sorted(undeclared_mmio))
        sys.stderr.write(
            f"[gen] CONTRACT_MMIO_MISMATCH: {args.source}: asm stores to {addrs} "
            f"but mmio_effects declares {contract.mmio_effects or '(none)'}. "
            "Add every hardware-register address to `// CONTRACT: mmio_effects:` "
            "(Pitfall 13/16, prompts/reverser_system.md) and re-run — a spike "
            "cannot prove correctness for an undeclared bus/VRAM/OAM/CGRAM effect.\n"
        )
        return 3

    # Safety net: the asm has an indexed store → the output address is
    # dynamic, the contract is necessarily wrong, force a manual spike.
    # EXCEPTION: a `// SPIKE_COMPARE: region` contract compares the whole WRAM
    # (stack-masked) rather than a fixed slot, so a dynamic output address is
    # fine — the region comparison observes wherever the store landed.
    if bridge_asm_has_indexed_store(contract.func_name) and not contract.compare_region:
        sys.stderr.write(
            f"[gen] {args.source}: asm contains an indexed store "
            "(sta/stx/sty/stz $XXXX,x|y) so the output address is dynamic. "
            "The LLM contract is overridden — treating this as CUSTOM_SPIKE.\n"
        )
        return 0

    suffix = args.spike_suffix
    spike_src = PARITY_SRC / f"spike_{contract.func_name.lower()}_{suffix}.c"
    spike_src.write_text(render_spike(text, contract))
    bin_name = add_makefile_rule(contract.func_name, suffix, contract.extra_src)
    print(f"[gen] wrote {spike_src.relative_to(ROOT)} (target {bin_name})")

    if args.build:
        res = subprocess.run(
            ["make", "-C", str(ROOT / "parity"), bin_name],
            capture_output=True, text=True,
        )
        if res.returncode != 0:
            sys.stderr.write(res.stderr)
            return res.returncode
        print(f"[gen] built parity/{bin_name}")

    if args.run > 0 and args.build:
        bin_path = ROOT / "parity" / bin_name
        try:
            res = subprocess.run(
                [str(bin_path), str(args.rom), str(args.run)],
                capture_output=True, text=True,
                # Hard cap: a spike that does not finish 100 trials in 20 s
                # contains an infinite loop. Kill it instead of orphaning it
                # under the parent's timeout.
                timeout=args.run_timeout,
            )
        except subprocess.TimeoutExpired:
            sys.stderr.write(
                f"[gen] spike binary exceeded {args.run_timeout}s — likely "
                "infinite loop in the translated C body.\n"
            )
            return 124  # standard shell convention for timeout
        print(res.stdout)
        sys.stderr.write(res.stderr)
        return res.returncode

    return 0


if __name__ == "__main__":
    sys.exit(main())
