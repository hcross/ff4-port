// AUTO-GENERATED parity spike for field::SetNpcMapPtr @ $00:C357.
// Generated 2026-10-10 from an ff4-gnw/field/NpcMapCells.c wrapper carrying this entry's CONTRACT (bundled body, no per-entry source); kept as the reproducible copy of the proof (strict 50000/0).
// DO NOT EDIT BY HAND — regenerate via:
//   python translator/generate_spike.py port/field/SetNpcMapPtr.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes.h"
#include "cpu.h"
#include "ppu.h"
#include "dma.h"

#define TARGET_ADDR_24 0x00C357u

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
// translation but not defined in port/field/SetNpcMapPtr.c).
// ---------------------------------------------------------------------------

// (no auto-emitted helpers)

// ---------------------------------------------------------------------------
// LLM-translated C function (verbatim copy from port/field/SetNpcMapPtr.c)
// ---------------------------------------------------------------------------

#include "snes/snes.h"

/* Field NPC-map cell writers + index computation — the $00:C2FF/$C347/$C357
 * cluster (~32 combined calls/frame on the field map, the second-hottest
 * interpreted group after CalcObjScreenPos).
 *
 * The NPC occupation map lives at $7F:4C00 (WRAM), indexed by
 * idx = y*32 + x. $00:C357 computes that index into $3D/$3E (16-bit,
 * little-endian) from the DP tile coords $0E (y) and $0C (x); the two
 * writers call it and then store through the long-indexed address.
 *
 * ROM-bytes-are-truth: the reference disassembly annotates the index
 * routine at $00:C355; the real entry is $00:C357 ($C355/56 = FA 60, the
 * PLX/RTS tail of $C347). Third off-by-2 of this class (D00F535, D00BDB2).
 *
 * DP is the caller's D (=$0600 in the field engine, probe-verified for all
 * three entries, mf=1 xf=0). $3D/$3E are real outputs of ALL three entries
 * (the writers update them via the shared computation).
 *
 * Exact asm semantics kept: the low index byte is ((y & 7) << 5) + x with
 * an 8-BIT add — a carry out of bit 7 is dropped, not propagated to $3E
 * (harmless in-game: the callers bound x/y to < $20 first, but the spike
 * fuzzes the full byte range, so match the truncation exactly). */

static uint16_t npc_map_index(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;
    const uint16_t dp = cpu->dp;
    const uint8_t y = ram[(uint16_t)(dp + 0x0E)];   // LDA $0E
    const uint8_t hi = (uint8_t)(y >> 3);            // 3x LSR $3E
    const uint8_t base = (uint8_t)((y & 7) << 5);    // 3x ROR $3D
    const uint8_t x = ram[(uint16_t)(dp + 0x0C)];
    const uint16_t sum = (uint16_t)(base + x);       // LDA $3D / CLC / ADC $0C (8-bit)
    const uint8_t lo = (uint8_t)sum;
    ram[(uint16_t)(dp + 0x3E)] = hi;
    ram[(uint16_t)(dp + 0x3D)] = lo;
    // Exit state of $C357 (strict, 2026-10-10): A.low = lo, NZVC from the adc.
    cpu->a = (uint16_t)((cpu->a & 0xFF00) | lo);
    cpu->c = sum > 0xFF;
    cpu->v = (~(base ^ x) & (base ^ lo) & 0x80) != 0;
    cpu->z = (lo == 0);
    cpu->n = (lo & 0x80) != 0;
    return (uint16_t)((hi << 8) | lo);
}

/* The two writers wrap the index call in PHX ... PLX: X survives, and PLX
 * (x=16) sets N/Z from it last; A.low is the stored byte. */
static void writer_exit(Cpu *cpu, uint8_t stored) {
    cpu->a = (uint16_t)((cpu->a & 0xFF00) | stored);
    cpu->z = (cpu->x == 0);
    cpu->n = (cpu->x & 0x8000) != 0;
}

/* $00:C357 — compute the NPC-map index into $3D/$3E. */
void SetNpcMapPtr_c(Snes *snes) {
    npc_map_index(snes);
}

/* $00:C2FF — clear the NPC-map cell at the current tile ($7F:4C00+idx = 0). */
void ClearNpcMapCell_c(Snes *snes) {
    const uint16_t idx = npc_map_index(snes);
    snes->ram[0x14C00 + idx] = 0x00;                 // STA $7F4C00,X (A=0)
    writer_exit(snes->cpu, 0x00);
}

/* $00:C347 — mark the NPC-map cell with the current object ($AE | $80). */
void SetNpcMapCell_c(Snes *snes) {
    const uint16_t dp = snes->cpu->dp;
    const uint16_t idx = npc_map_index(snes);
    const uint8_t v = (uint8_t)(snes->ram[(uint16_t)(dp + 0xAE)] | 0x80);
    snes->ram[0x14C00 + idx] = v;
    writer_exit(snes->cpu, v);
}


// ---------------------------------------------------------------------------
// Common helpers (same shape as the manual M2/M3/M4 spikes)
// ---------------------------------------------------------------------------

static uint8_t *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *buf = malloc(sz);
    if (fread(buf, 1, sz, f) != (size_t)sz) { free(buf); fclose(f); return NULL; }
    fclose(f); *out_len = sz; return buf;
}

// Translation-patch variant comparison (registry/patch_impact.py, ADR-008):
// swap the cart's ROM buffer to a variant's canonical image AFTER the
// vanilla baseline is already captured. cart_load() (see LakeSnes/snes/
// cart.c) touches ONLY cart->{type,rom,romSize,ram,ramSize} -- unlike
// snes_loadRom(), which calls snes_reset() internally and would destroy
// the very WRAM/CPU baseline this swap exists to preserve. Boot and
// baseline therefore ALWAYS come from argv[1] (vanilla in every documented
// invocation); only the CODE BYTES the interpreter reads from here on
// change. Without this, comparing a not-gated routine's spike against a
// variant image would inherit that variant's OWN (possibly patched) boot
// sequence as a confound -- e.g. Mult8_c's undeclared entry carry flag
// silently differing because the variant's early-boot code differs,
// producing mass false "diverged" verdicts unrelated to the routine
// itself (found empirically 2026-07-16: 37/122 false positives before
// this fix).
static void swap_variant_rom(Snes *snes, const char *path) {
    size_t len = 0;
    uint8_t *bytes = read_file(path, &len);
    if (!bytes) { fprintf(stderr, "swap_variant_rom: failed to read %s\n", path); exit(4); }
    if (len == 0 || (len & (len - 1)) != 0) {
        fprintf(stderr, "swap_variant_rom: %s size %zu is not a power of 2\n", path, len);
        exit(4);
    }
    cart_load(snes->cart, snes->cart->type, bytes, (int)len, snes->cart->ramSize);
    free(bytes);
}

static inline uint32_t stack_addr(const Cpu *cpu) {
    return cpu->e ? (0x0100u | (cpu->sp & 0xFFu)) : cpu->sp;
}

// Firmware semantics (ff4-gnw snes/cpu.c run_emulated_func): push a 3-byte
// JSL-style frame and stop once 2 (RTS) or 3 (RTL) of those bytes have been
// pulled, then restore SP. The former 2-byte JSR frame never returned from an
// RTL-terminated routine (every JSL entry: *_ext, bank-$15 field code): the
// asm side ran on into arbitrary bytes until max_ops (found 2026-10-10 on
// _15c23d, a bare `rtl`, whose spike diverged on $0002 and the PPU regs).
static void run_emulated_func(Snes *snes, uint32_t pc24) {
    Cpu *cpu = snes->cpu;
    uint16_t sp_save = cpu->sp;
    snes->ram[stack_addr(cpu)] = 0x12; cpu->sp--;
    snes->ram[stack_addr(cpu)] = 0x34; cpu->sp--;
    snes->ram[stack_addr(cpu)] = 0x56; cpu->sp--;
    cpu->k = (uint8_t)(pc24 >> 16);
    cpu->pc = (uint16_t)(pc24 & 0xFFFF);
    long max_ops = 200000;
    while ((uint16_t)(sp_save - cpu->sp) >= 2u && max_ops-- > 0) {
        cpu_runOpcode(cpu);
        if (cpu->waiting || cpu->stopped) break;
    }
    cpu->sp = sp_save;
}

typedef struct {
    uint8_t ram[0x20000];
    uint16_t a, x, y, sp, pc, dp;
    uint8_t k, db;
    bool c, z, v, n, i, d, xf, mf, e;
    bool waiting, stopped, irqWanted, nmiWanted;
} Snap;

static void snap_take(Snap *s, Snes *snes) {
    memcpy(s->ram, snes->ram, sizeof(s->ram));
    Cpu *c = snes->cpu;
    s->a=c->a; s->x=c->x; s->y=c->y; s->sp=c->sp; s->pc=c->pc; s->dp=c->dp;
    s->k=c->k; s->db=c->db;
    s->c=c->c; s->z=c->z; s->v=c->v; s->n=c->n;
    s->i=c->i; s->d=c->d; s->xf=c->xf; s->mf=c->mf; s->e=c->e;
    s->waiting=c->waiting; s->stopped=c->stopped;
    s->irqWanted=c->irqWanted; s->nmiWanted=c->nmiWanted;
}
static void snap_restore(const Snap *s, Snes *snes) {
    memcpy(snes->ram, s->ram, sizeof(s->ram));
    Cpu *c = snes->cpu;
    c->a=s->a; c->x=s->x; c->y=s->y; c->sp=s->sp; c->pc=s->pc; c->dp=s->dp;
    c->k=s->k; c->db=s->db;
    c->c=s->c; c->z=s->z; c->v=s->v; c->n=s->n;
    c->i=s->i; c->d=s->d; c->xf=s->xf; c->mf=s->mf; c->e=s->e;
    c->waiting=s->waiting; c->stopped=s->stopped;
    c->irqWanted=s->irqWanted; c->nmiWanted=s->nmiWanted;
}

uint16_t read16(const uint8_t *ram, int addr) {
    return (uint16_t)(ram[addr] | (ram[addr + 1] << 8));
}
void write16(uint8_t *ram, int addr, uint16_t v) {
    ram[addr] = v & 0xFF; ram[addr + 1] = (v >> 8) & 0xFF;
}

// ---------------------------------------------------------------------------
// Strict full-state diff (FF4_SPIKE_STRICT) -- see main()
// ---------------------------------------------------------------------------

enum { STRICT_WRAM, STRICT_A, STRICT_X, STRICT_Y, STRICT_DP, STRICT_DB,
       STRICT_MF, STRICT_XF, STRICT_C, STRICT_Z, STRICT_V, STRICT_N,
       STRICT_VRAM, STRICT_CGRAM, STRICT_OAM, STRICT_PPUREG, STRICT_DMAREG,
       STRICT_NFIELDS };
static const char *strict_field_name[STRICT_NFIELDS] = {
    "wram", "a", "x", "y", "dp", "db", "mf", "xf", "c", "z", "v", "n",
    "vram", "cgram", "oam", "ppureg", "dmareg" };

/* Hardware side of the strict compare: snap_restore() only rewinds WRAM and
 * the CPU, so the bus-visible state a DMA/MMIO routine produces (VRAM, CGRAM,
 * OAM, the PPU access pointers, the DMA channel registers it leaves behind)
 * is saved/restored around the two runs here. Without it a manual-loop C
 * port of a DMA routine "passes" a WRAM-only spike by construction (the
 * TfrVRAM_c / DMA_TRIGGER class, F6/F15). */
typedef struct {
    uint16_t vram[0x8000], cgram[0x100], oam[0x100];
    uint8_t highOam[0x20];
    uint16_t vramPointer, vramIncrement;
    uint8_t cgramPointer, oamAdr;
    DmaChannel ch[8];
} HwSnap;

static void hw_take(HwSnap *h, Snes *snes) {
    Ppu *p = snes->ppu;
    memcpy(h->vram, p->vram, sizeof(h->vram));
    memcpy(h->cgram, p->cgram, sizeof(h->cgram));
    memcpy(h->oam, p->oam, sizeof(h->oam));
    memcpy(h->highOam, p->highOam, sizeof(h->highOam));
    h->vramPointer = p->vramPointer; h->vramIncrement = p->vramIncrement;
    h->cgramPointer = p->cgramPointer; h->oamAdr = p->oamAdr;
    memcpy(h->ch, snes->dma->channel, sizeof(h->ch));
}

static void hw_restore(const HwSnap *h, Snes *snes) {
    Ppu *p = snes->ppu;
    memcpy(p->vram, h->vram, sizeof(h->vram));
    memcpy(p->cgram, h->cgram, sizeof(h->cgram));
    memcpy(p->oam, h->oam, sizeof(h->oam));
    memcpy(p->highOam, h->highOam, sizeof(h->highOam));
    p->vramPointer = h->vramPointer; p->vramIncrement = h->vramIncrement;
    p->cgramPointer = h->cgramPointer; p->oamAdr = h->oamAdr;
    memcpy(snes->dma->channel, h->ch, sizeof(h->ch));
}

static int dma_regs_differ(const DmaChannel *x, const DmaChannel *y) {
    for (int i = 0; i < 8; i++)
        if (x[i].bAdr != y[i].bAdr || x[i].aAdr != y[i].aAdr || x[i].aBank != y[i].aBank ||
            x[i].size != y[i].size || x[i].mode != y[i].mode || x[i].fixed != y[i].fixed ||
            x[i].decrement != y[i].decrement || x[i].fromB != y[i].fromB) return 1;
    return 0;
}

static void strict_hw_diff(const HwSnap *x, const HwSnap *y, long *hist, int *any) {
#define HW_CHK(F, cond) do { if (cond) { hist[F]++; *any = 1; } } while (0)
    HW_CHK(STRICT_VRAM, memcmp(x->vram, y->vram, sizeof(x->vram)) != 0);
    HW_CHK(STRICT_CGRAM, memcmp(x->cgram, y->cgram, sizeof(x->cgram)) != 0);
    HW_CHK(STRICT_OAM, memcmp(x->oam, y->oam, sizeof(x->oam)) != 0 ||
                       memcmp(x->highOam, y->highOam, sizeof(x->highOam)) != 0);
    HW_CHK(STRICT_PPUREG, x->vramPointer != y->vramPointer || x->vramIncrement != y->vramIncrement ||
                          x->cgramPointer != y->cgramPointer || x->oamAdr != y->oamAdr);
    HW_CHK(STRICT_DMAREG, dma_regs_differ(x->ch, y->ch));
#undef HW_CHK
}

/* Strict mode also seeds the registers the CONTRACT does NOT declare as
 * inputs with per-trial noise (identical on both sides), instead of the
 * fixed 0 the contract path uses -- otherwise "the C never sets X=0 on exit"
 * is invisible whenever X happened to be 0 at entry. Index registers stay
 * 8-bit-clean under xf=1, as on hardware. */
static int strict_noise;
static uint16_t strict_na, strict_nx, strict_ny;
static void strict_apply_noise(Cpu *c) {
    if (!strict_noise) return;
    c->a = strict_na;
    c->x = c->xf ? (strict_nx & 0xFF) : strict_nx;
    c->y = c->xf ? (strict_ny & 0xFF) : strict_ny;
}

static int strict_wram_masked(int a, uint16_t pre_sp) {
    int sp_page = pre_sp & 0xFF00;
    if (a >= sp_page && a < sp_page + 0x100) return 1;
    return 0;
}

static int strict_first_wram_diff(const Snap *x, const Snap *y, uint16_t pre_sp) {
    for (int a = 0; a < 0x20000; a++)
        if (!strict_wram_masked(a, pre_sp) && x->ram[a] != y->ram[a]) return a;
    return -1;
}

static int strict_diff(const Snap *x, const Snap *y, uint16_t pre_sp, long *hist) {
    int any = 0;
#define STRICT_CHK(F, cond) do { if (cond) { hist[F]++; any = 1; } } while (0)
    STRICT_CHK(STRICT_WRAM, strict_first_wram_diff(x, y, pre_sp) >= 0);
    STRICT_CHK(STRICT_A, x->a != y->a);
    STRICT_CHK(STRICT_X, x->x != y->x);
    STRICT_CHK(STRICT_Y, x->y != y->y);
    STRICT_CHK(STRICT_DP, x->dp != y->dp);
    STRICT_CHK(STRICT_DB, x->db != y->db);
    STRICT_CHK(STRICT_MF, x->mf != y->mf);
    STRICT_CHK(STRICT_XF, x->xf != y->xf);
    STRICT_CHK(STRICT_C, x->c != y->c);
    STRICT_CHK(STRICT_Z, x->z != y->z);
    STRICT_CHK(STRICT_V, x->v != y->v);
    STRICT_CHK(STRICT_N, x->n != y->n);
#undef STRICT_CHK
    return any;
}

static void strict_print(int trial, const Snap *x, const Snap *y, uint16_t pre_sp) {
    int w = strict_first_wram_diff(x, y, pre_sp);
    printf("strict %4d : a=%04X/%04X x=%04X/%04X y=%04X/%04X mf=%d/%d xf=%d/%d "
           "czvn=%d%d%d%d/%d%d%d%d",
           trial, x->a, y->a, x->x, y->x, x->y, y->y, x->mf, y->mf, x->xf, y->xf,
           x->c, x->z, x->v, x->n, y->c, y->z, y->v, y->n);
    if (w >= 0) printf(" wram@$%05X=%02X/%02X", w, x->ram[w], y->ram[w]);
    printf("  (asm/c)\n");
}

// ---------------------------------------------------------------------------
// asm wrapper from the contract
// ---------------------------------------------------------------------------

static void run_asm(Snes *snes, uint8_t arg_a, uint8_t arg_x) {
    Cpu *c = snes->cpu;
    c->dp = 0x0600;
    c->db = 0x00;
    c->mf = true;
    c->xf = false;
    c->a = 0; c->x = 0; c->y = 0;
    strict_apply_noise(c);
    c->a = arg_a;
    c->x = arg_x;
    c->z = (arg_a == 0);
    c->n = ((arg_a & 0x8000) != 0);
    run_emulated_func(snes, TARGET_ADDR_24);
}

// ---------------------------------------------------------------------------
// Host PRNG
// ---------------------------------------------------------------------------

static uint32_t host_rng_state = 0xC0FFEEu;
static uint32_t host_rng(void) {
    uint32_t x = host_rng_state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    host_rng_state = x; return x;
}

// ---------------------------------------------------------------------------
// Trial driver
// ---------------------------------------------------------------------------

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <rom.sfc> [n_trials] [variant_rom.sfc]\n", argv[0]);
        return 1;
    }
    int n_trials = (argc >= 3) ? atoi(argv[2]) : 1000;

    size_t rom_len = 0;
    uint8_t *rom = read_file(argv[1], &rom_len);
    if (!rom) return 2;

    Snes *snes = snes_init();
    if (!snes_loadRom(snes, rom, rom_len)) { fprintf(stderr, "loadRom fail\n"); return 3; }
    snes_reset(snes, true);
    for (int i = 0; i < 60; i++) snes_runFrame(snes);
    snes->cpu->i = true; snes->cpu->nmiWanted = false; snes->cpu->irqWanted = false;

    Snap baseline;
    snap_take(&baseline, snes);

    // Optional variant-comparison mode: see swap_variant_rom's comment.
    // argv[1] is still what booted and produced `baseline` above -- only
    // the ROM bytes the interpreter reads from here on change.
    if (argc >= 4) swap_variant_rom(snes, argv[3]);

    fprintf(stderr, "SetNpcMapPtr asm vs C : %d trials\n", n_trials);

    // Strict full-state mode (opt-in, FF4_SPIKE_STRICT=1|gate): in addition
    // to the contract compare above, diff the WHOLE post-call state -- WRAM
    // (stack page + SPIKE_MASK excluded), VRAM/CGRAM/OAM + PPU pointers + DMA
    // channel registers, and every CPU register/flag the
    // dispatch hook leaves to the caller (cpu.c does no state fixup after a
    // dispatched body). The contract compare alone observes one slot, or the
    // registers, or the WRAM -- never all three -- so a body can pass it while
    // leaving A/X/m/a neighbouring byte different from the asm (found
    // 2026-10-10 on Div16_c: remainder, rotated dividend, A=D, X=0, m=8 all
    // unobserved). =1 reports only; =gate also fails the exit code.
    const char *strict_env = getenv("FF4_SPIKE_STRICT");
    int strict = strict_env && *strict_env && strcmp(strict_env, "0") != 0;
    int strict_gate = strict && strcmp(strict_env, "gate") == 0;
    static Snap strict_asm, strict_c;
    static HwSnap hw_pre, hw_asm, hw_c;
    int strict_fails = 0;
    long strict_hist[STRICT_NFIELDS] = {0};
    int strict_first_wram = -1;

    int fails = 0;
    for (int trial = 0; trial < n_trials; trial++) {
        snap_restore(&baseline, snes);

        // Randomise inputs.
        snes->ram[0x060C] = (uint8_t)host_rng();
        snes->ram[0x060E] = (uint8_t)host_rng();
        snes->ram[0x06AE] = (uint8_t)host_rng();
        uint8_t arg_a = (uint8_t)host_rng();
        uint8_t arg_x = (uint8_t)host_rng();

        if (strict) {
            strict_noise = 1;
            strict_na = (uint16_t)host_rng(); strict_nx = (uint16_t)host_rng();
            strict_ny = (uint16_t)host_rng();
        }

        // Pre-call snapshot for replaying C with the exact same state.
        Snap pre;
        snap_take(&pre, snes);

        // Run ASM
        if (strict) hw_take(&hw_pre, snes);
        run_asm(snes, arg_a, arg_x);
        Snap asm_post; snap_take(&asm_post, snes);
        if (strict) { snap_take(&strict_asm, snes); hw_take(&hw_asm, snes); hw_restore(&hw_pre, snes); }

        // Restore and run C
        snap_restore(&pre, snes);
        snes->cpu->dp = 0x0600;
        snes->cpu->db = 0x00;
        snes->cpu->mf = true;
        snes->cpu->xf = false;
        snes->cpu->a = 0; snes->cpu->x = 0; snes->cpu->y = 0;
        strict_apply_noise(snes->cpu);
        snes->cpu->a = arg_a;
        snes->cpu->x = arg_x;
        SetNpcMapPtr_c(snes);
        Snap c_post; snap_take(&c_post, snes);
        if (strict) { snap_take(&strict_c, snes); hw_take(&hw_c, snes); }

        /* Mask the stack page relative to SP: the asm's push/pull and
         * run_emulated_func's return frame mutate it, the inlined C never
         * touches the stack. FF4's combat stack lives in $02xx (SP~$02E5),
         * the field stack in $01xx — so derive the page from pre.sp. */
        int sp_page = pre.sp & 0xFF00;
        int diff = -1;
        for (int a = 0; a < 0x20000; a++) {
            if (a >= sp_page && a < sp_page + 0x100) continue;  /* mask stack page */
            if (asm_post.ram[a] != c_post.ram[a]) { diff = a; break; }
        }
        bool ok = (diff < 0);
        if (!ok) {
            printf("trial %4d : first WRAM diff at $%05X  asm=%02X c=%02X  FAIL\n",
                   trial, diff, asm_post.ram[diff], c_post.ram[diff]);
            fails++;
        }
        if (strict) {
            int w = strict_diff(&strict_asm, &strict_c, pre.sp, strict_hist);
            strict_hw_diff(&hw_asm, &hw_c, strict_hist, &w);
            if (w) {
                strict_fails++;
                if (strict_first_wram < 0 && strict_hist[STRICT_WRAM] > 0)
                    strict_first_wram = strict_first_wram_diff(&strict_asm, &strict_c, pre.sp);
                if (strict_fails <= 3) strict_print(trial, &strict_asm, &strict_c, pre.sp);
            }
        }
    }

    printf("\n=== summary === trials: %d, fails: %d\n", n_trials, fails);
    if (strict) {
        printf("=== strict === trials: %d, strict_fails: %d, fields:", n_trials, strict_fails);
        for (int f = 0; f < STRICT_NFIELDS; f++)
            if (strict_hist[f]) printf(" %s=%ld", strict_field_name[f], strict_hist[f]);
        if (strict_first_wram >= 0) printf(" first_wram=$%05X", strict_first_wram);
        printf("\n");
    }
    free(rom);
    if (strict_gate && strict_fails) return 1;
    return fails == 0 ? 0 : 1;
}
