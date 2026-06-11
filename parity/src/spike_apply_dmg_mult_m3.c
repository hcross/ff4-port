// Spike M3 — ApplyDmgMult asm vs C comparison.
//
// Call site context (from CalcDmg @c9c5):
//   - A 8-bit mode (hypothesis to validate via the test)
//   - X 16-bit mode (xf=0 inherited from clr_ax)
//   - DB = $7E, DP = 0
//   - A = elemental_multiplier (8-bit)
//   - $a4-$a5 = damage 16-bit little-endian (lo, hi)
//
// ApplyDmgMult routine (damage.asm @ca41):
//   bne @ca48          ; A != 0 ?
//   stz $a4            ; A==0 case: damage = 0
//   stz $a5
//   rts
//   @ca48:
//   lsr                ; A >>= 1
//   bne @ca50          ; A_orig > 1 ?
//   lsr $a5            ; A_orig == 1 case: damage >>= 1
//   ror $a4
//   rts
//   @ca50:
//   tax                ; X = A (mult)
//   stx $393d          ; Mult16 arg1 (16-bit because X is 16-bit)
//   ldx $a4            ; X = damage 16-bit
//   stx $393f          ; Mult16 arg2
//   jsr Mult16
//   ldx $3941          ; X = result lo
//   stx $a4            ; damage = result lo (truncated to 16-bit)
//   rts
//
// We compare the post-call output (damage) between the two implementations
// across 1000 fuzz trials. If failures appear → next hypothesis to test is
// "A 16-bit at entry".
//
// Usage:
//   ./ff4-spike-apply-dmg-mult <rom.sfc> [n_trials]

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes.h"
#include "cpu.h"

#define APPLY_DMG_MULT_ADDR_24 0x03CA41u
#define MULT16_ADDR_24         0x0383B9u   // 0x038000 (battle_code) + 0x03B9

#define RAM_DAMAGE_LO 0xA4
#define RAM_DAMAGE_HI 0xA5
#define RAM_MULT16_ARG1 0x393D   // 16-bit
#define RAM_MULT16_ARG2 0x393F   // 16-bit
#define RAM_MULT16_RESLO 0x3941  // low 16-bit of the 32-bit result

// ---------------------------------------------------------------------------
// Common helpers (shared with M1/M2 — should be factored into a header one day)
// ---------------------------------------------------------------------------

static uint8_t *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = malloc(sz);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, sz, f) != (size_t)sz) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *out_len = sz;
    return buf;
}

static inline uint32_t stack_addr(const Cpu *cpu) {
    return cpu->e ? (0x0100u | (cpu->sp & 0xFFu)) : cpu->sp;
}

static void run_emulated_func(Snes *snes, uint32_t pc24) {
    Cpu *cpu = snes->cpu;
    uint16_t sp_save = cpu->sp;
    snes->ram[stack_addr(cpu)] = 0x12; cpu->sp--;
    snes->ram[stack_addr(cpu)] = 0x34; cpu->sp--;
    cpu->k = (uint8_t)(pc24 >> 16);
    cpu->pc = (uint16_t)(pc24 & 0xFFFF);
    long max_ops = 100000;
    while (cpu->sp != sp_save && max_ops-- > 0) {
        cpu_runOpcode(cpu);
        if (cpu->waiting || cpu->stopped) break;
    }
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

// ---------------------------------------------------------------------------
// C side: ApplyDmgMult reimplemented under the A 8-bit / X 16-bit hypothesis.
//
// `mult16_emu` runs Mult16 via RunEmulatedFunc with A 16-bit (Mult16 starts
// with `longa`) and X 16-bit. Inputs are already in RAM at the right addresses.
// ---------------------------------------------------------------------------

static void mult16_emu(Snes *snes) {
    Cpu *c = snes->cpu;
    uint16_t saved_a = c->a, saved_x = c->x, saved_y = c->y;
    uint16_t saved_sp = c->sp, saved_pc = c->pc, saved_dp = c->dp;
    uint8_t saved_k = c->k, saved_db = c->db;
    bool saved_mf = c->mf, saved_xf = c->xf;

    c->dp = 0;
    c->db = 0x7E;
    c->mf = false;  // A 16-bit (Mult16 issues `longa` itself, but be explicit)
    c->xf = false;  // X 16-bit (Mult16 uses `ldx #$10`)

    run_emulated_func(snes, MULT16_ADDR_24);

    c->a = saved_a; c->x = saved_x; c->y = saved_y;
    c->sp = saved_sp; c->pc = saved_pc; c->dp = saved_dp;
    c->k = saved_k; c->db = saved_db;
    c->mf = saved_mf; c->xf = saved_xf;
}

static inline uint16_t read16(const uint8_t *ram, int addr) {
    return (uint16_t)(ram[addr] | (ram[addr + 1] << 8));
}
static inline void write16(uint8_t *ram, int addr, uint16_t v) {
    ram[addr] = v & 0xFF;
    ram[addr + 1] = (v >> 8) & 0xFF;
}

static void ApplyDmgMult_c(Snes *snes, uint8_t mult) {
    uint8_t *ram = snes->ram;

    if (mult == 0) {
        // bne @ca48 → not taken: zero the damage
        ram[RAM_DAMAGE_LO] = 0;
        ram[RAM_DAMAGE_HI] = 0;
        return;
    }
    // @ca48: A >>= 1 ; bne @ca50 → if zero, A_orig was 1
    uint8_t shifted = mult >> 1;
    if (shifted == 0) {
        // mult was 1: damage 16-bit >>= 1
        uint16_t dmg = read16(ram, RAM_DAMAGE_LO);
        dmg >>= 1;
        write16(ram, RAM_DAMAGE_LO, dmg);
        return;
    }
    // @ca50: Mult16(shifted, damage), result truncated to 16-bit.
    // In X 16-bit mode, `stx $393d` writes `shifted` at $393d (lo) and 0 at
    // $393e (X_hi was zeroed by clr_ax at the top of CalcDmg, and `tax`
    // preserves that — only A_lo transfers in A 8-bit mode; X_lo takes A_lo,
    // X_hi stays at 0).
    write16(ram, RAM_MULT16_ARG1, (uint16_t)shifted);
    uint16_t dmg = read16(ram, RAM_DAMAGE_LO);
    write16(ram, RAM_MULT16_ARG2, dmg);

    mult16_emu(snes);

    // damage = result lo (the high bits of the 32-bit product are discarded)
    uint16_t result_lo = read16(ram, RAM_MULT16_RESLO);
    write16(ram, RAM_DAMAGE_LO, result_lo);
}

// ---------------------------------------------------------------------------
// ASM side: a wrapper that sets DB/DP/M/X and calls ApplyDmgMult.
// ---------------------------------------------------------------------------

static void ApplyDmgMult_asm(Snes *snes, uint8_t mult) {
    Cpu *c = snes->cpu;
    c->dp = 0;
    c->db = 0x7E;
    c->mf = true;   // A 8-bit (hypothesis)
    c->xf = false;  // X 16-bit
    c->a = mult;    // input
    c->x = 0;       // matches clr_ax (X_hi = 0)
    c->y = 0;
    // CLASSIC PITFALL: the routine starts with `bne @ca48` which reads Z.
    // In the normal flow `lda $38FE` JUST BEFORE `jsr ApplyDmgMult` sets Z.
    // We jump directly to @ca41 without an LDA, so we must simulate the
    // entry flags as if we had just done `lda mult`.
    c->z = (mult == 0);
    c->n = (mult & 0x80) != 0;
    run_emulated_func(snes, APPLY_DMG_MULT_ADDR_24);
}

// ---------------------------------------------------------------------------
// Host PRNG
// ---------------------------------------------------------------------------

static uint32_t host_rng_state = 0xDEADBEEFu;
static uint32_t host_rng(void) {
    uint32_t x = host_rng_state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    host_rng_state = x;
    return x;
}

// ---------------------------------------------------------------------------

typedef struct {
    uint8_t mult;
    uint16_t damage_in;
} Trial;

static int run_trial(Snes *snes, const Trial *t, Snap *baseline,
                     int trial_id, bool verbose) {
    snap_restore(baseline, snes);
    write16(snes->ram, RAM_DAMAGE_LO, t->damage_in);

    Snap pre;
    snap_take(&pre, snes);

    // ASM
    ApplyDmgMult_asm(snes, t->mult);
    uint16_t out_asm = read16(snes->ram, RAM_DAMAGE_LO);

    // C
    snap_restore(&pre, snes);
    ApplyDmgMult_c(snes, t->mult);
    uint16_t out_c = read16(snes->ram, RAM_DAMAGE_LO);

    bool ok = (out_asm == out_c);
    if (!ok || verbose) {
        printf("trial %4d : mult=%3u damage_in=%5u  asm=%5u c=%5u  %s\n",
               trial_id, t->mult, t->damage_in, out_asm, out_c, ok ? "OK" : "FAIL");
    }
    return ok ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <rom.sfc> [n_trials]\n", argv[0]);
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
    snes->cpu->i = true;
    snes->cpu->nmiWanted = false;
    snes->cpu->irqWanted = false;

    Snap baseline;
    snap_take(&baseline, snes);

    fprintf(stderr, "comparing ApplyDmgMult asm vs C : %d trials (A 8-bit / X 16-bit hypothesis)\n", n_trials);

    // Named trials: exercise each of the three branches of the function.
    int fails = 0;
    Trial named[] = {
        {0,   0x1234},  // mult=0 → damage=0
        {0,   0xFFFF},  // mult=0 → damage=0
        {1,   0x1000},  // mult=1 → damage >>= 1 = 0x0800
        {1,   0x0001},  // mult=1 → damage >>= 1 = 0
        {2,   0x0100},  // mult>1 → Mult16(1, damage), result lo = 256
        {2,   0x4000},  // mult>1 → Mult16(1, damage) = 0x4000
        {255, 0x0010},  // mult=255 → Mult16(127, 0x10), result = 0x07F0
        {3,   0x5555},  // arbitrary
    };
    for (size_t i = 0; i < sizeof(named)/sizeof(named[0]); i++) {
        fails += run_trial(snes, &named[i], &baseline, (int)i, /*verbose*/true);
    }

    printf("\n--- named trials done : %d fail(s) ---\n\n", fails);

    int fuzz_fails = 0;
    for (int i = 0; i < n_trials; i++) {
        Trial t = { .mult = (uint8_t)(host_rng() & 0xFF),
                    .damage_in = (uint16_t)(host_rng() & 0xFFFF) };
        fuzz_fails += run_trial(snes, &t, &baseline, i, /*verbose*/false);
    }

    printf("\n=== summary ===\nnamed fails : %d\nfuzz trials : %d\nfuzz fails  : %d\n",
           fails, n_trials, fuzz_fails);
    free(rom);
    return (fails + fuzz_fails) == 0 ? 0 : 1;
}
