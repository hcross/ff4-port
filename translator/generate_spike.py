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


_CONTRACT_RE = re.compile(r"//\s*CONTRACT:\s*\n((?:\s*//\s*[^\n]*\n)+)", re.MULTILINE)
_CUSTOM_RE = re.compile(r"//\s*CUSTOM_SPIKE:\s*yes", re.IGNORECASE)
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
    if source_path is not None and source_path.parent.parent.name == "port":
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
        sys.stderr.write(f"[gen] ca65-bridge could not resolve {module}::{name}\n")
        return None

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
            if k in out:
                out[k] = None if v.lower() == "none" else int(v)
        return out

    def parse_ram(spec: str) -> list[tuple[int, int]]:
        # "0x38FA=1, 0x38FB=1"
        out: list[tuple[int, int]] = []
        if not spec or spec.lower() == "none":
            return out
        for part in spec.split(","):
            part = part.strip()
            if "=" not in part:
                continue
            addr, width = [s.strip() for s in part.split("=", 1)]
            out.append((int(addr, 16), int(width)))
        return out

    def parse_single_ram(spec: str) -> Optional[tuple[int, int]]:
        ram = parse_ram(spec)
        return ram[0] if ram else None

    def parse_mode(spec: str, key: str) -> bool:
        # "mf=true, xf=false, dp=0x0, db=0x7E"
        for part in spec.split(","):
            part = part.strip()
            if part.lower().startswith(key + "="):
                return part.split("=", 1)[1].strip().lower() == "true"
        return True  # default A 8-bit

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
    entry_flags = find("entry_flags") or ""
    entry_z = parse_flag(entry_flags, "z")
    entry_n = parse_flag(entry_flags, "n")

    return Contract(
        func_name=name,
        module=module,
        addr24=addr24,
        inputs_reg=inputs_reg,
        inputs_ram=inputs_ram,
        output_ram=output_ram,
        entry_mf=entry_mf,
        entry_xf=entry_xf,
        entry_z=entry_z,
        entry_n=entry_n,
        custom_spike=custom_spike,
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
static inline uint16_t read16(const uint8_t *ram, int addr);
static inline void write16(uint8_t *ram, int addr, uint16_t v);

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

static inline uint16_t read16(const uint8_t *ram, int addr) {{
    return (uint16_t)(ram[addr] | (ram[addr + 1] << 8));
}}
static inline void write16(uint8_t *ram, int addr, uint16_t v) {{
    ram[addr] = v & 0xFF; ram[addr + 1] = (v >> 8) & 0xFF;
}}

// ---------------------------------------------------------------------------
// asm wrapper from the contract
// ---------------------------------------------------------------------------

static void run_asm(Snes *snes{asm_args_decl}) {{
    Cpu *c = snes->cpu;
    c->dp = 0;
    c->db = 0x7E;
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

        bool ok = (out_asm == out_c);
        if (!ok) {{
            printf("trial %4d : asm=%u c=%u  FAIL\n", trial, out_asm, out_c);
            fails++;
        }}
    }}

    printf("\n=== summary === trials: %d, fails: %d\n", n_trials, fails);
    free(rom);
    return fails == 0 ? 0 : 1;
}}
"""


def render_spike(c_translation: str, contract: Contract) -> str:
    bank = (contract.addr24 >> 16) & 0xFF
    offset = contract.addr24 & 0xFFFF

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

    # Output capture
    if contract.output_ram is None:
        # No single-output contract; fall back to "no comparison" — caller must
        # add a CUSTOM_SPIKE marker. We still generate the wrapper for runs.
        capture_asm_output = "        uint16_t out_asm = 0; (void)snes;"
        capture_c_output = "        uint16_t out_c = 0;"
    else:
        addr, width = contract.output_ram
        if width == 1:
            capture_asm_output = f"        uint8_t out_asm = snes->ram[0x{addr:04X}];"
            capture_c_output = f"        uint8_t out_c = snes->ram[0x{addr:04X}];"
        else:
            capture_asm_output = f"        uint16_t out_asm = read16(snes->ram, 0x{addr:04X});"
            capture_c_output = f"        uint16_t out_c = read16(snes->ram, 0x{addr:04X});"

    # C call line
    c_call = f"        {contract.func_name}_c(snes" + \
             ("" if not c_args else ", " + ", ".join(c_args)) + ");"

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
        ram_input_setup="\n".join(ram_input_setup_lines) or "        // no RAM inputs",
        reg_input_random="\n".join(reg_input_random_lines),
        c_call=c_call,
        capture_asm_output=capture_asm_output,
        capture_c_output=capture_c_output,
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


def add_makefile_rule(name: str) -> str:
    """Append a build rule for the auto spike. Returns the binary name."""
    slug = name.lower()
    bin_name = f"ff4-spike-auto-{slug}"
    text = PARITY_MAKEFILE.read_text()
    if bin_name in text:
        return bin_name
    rule = f"""
# AUTO: spike for {name}
{bin_name}: src/spike_{slug}_auto.c $(CORE_SRC)
\t$(CC) $(CFLAGS) -o $@ src/spike_{slug}_auto.c $(CORE_SRC) $(LDFLAGS)
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

    spike_src = PARITY_SRC / f"spike_{contract.func_name.lower()}_auto.c"
    spike_src.write_text(render_spike(text, contract))
    bin_name = add_makefile_rule(contract.func_name)
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
        res = subprocess.run([str(bin_path), str(args.rom), str(args.run)],
                             capture_output=True, text=True)
        print(res.stdout)
        sys.stderr.write(res.stderr)
        return res.returncode

    return 0


if __name__ == "__main__":
    sys.exit(main())
