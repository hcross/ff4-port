// Spike M2 — asm vs C comparison of CalcHits with partial shadow-exec.
//
// Architecture:
//   - A single LakeSnes instance.
//   - ASM side: we set PC on CalcHits @ $03:C987 and let the asm run
//     (Rand99 is invoked from within via JSR).
//   - C side: CalcHits_c(snes) reproduces the damage.asm logic in C, and
//     for each Rand99 draw it calls RunEmulatedFunc($03:858B) which
//     executes the asm Rand99 and returns the value in cpu->a. This is
//     EXACTLY the zelda3 pattern of "a translated function delegates to
//     the asm for sub-routines that have not been translated yet".
//
//   - For each trial: snapshot RAM + regs → run asm → record output → restore
//     → run C → record output → compare byte-equal on $38FD.
//
// If 1000/1000 trials show CalcHits_asm == CalcHits_c, the snesrev
// shadow-exec architecture is definitively validated for FF4.
//
// Usage:
//   ./ff4-spike-calchits-m2 <rom.sfc> [n_trials]

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "snes.h"
#include "cpu.h"

#define CALCHITS_ADDR_24 0x03C987u
#define RAND99_ADDR_24   0x03858Bu

#define RAM_RNG_INDEX  0x0097
#define RAM_RNG_TABLE  0x1900
#define RAM_HIT_RATE   0x38FA
#define RAM_BASE_HITS  0x38FB
#define RAM_NHITS_OUT  0x38FD

// ---------------------------------------------------------------------------
// Helpers
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

// ---------------------------------------------------------------------------
// Snapshot / restore — RAM + CPU regs.
// A single buffer is allocated up front.
// ---------------------------------------------------------------------------

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
    s->a = c->a; s->x = c->x; s->y = c->y;
    s->sp = c->sp; s->pc = c->pc; s->dp = c->dp;
    s->k = c->k; s->db = c->db;
    s->c = c->c; s->z = c->z; s->v = c->v; s->n = c->n;
    s->i = c->i; s->d = c->d; s->xf = c->xf; s->mf = c->mf; s->e = c->e;
    s->waiting = c->waiting; s->stopped = c->stopped;
    s->irqWanted = c->irqWanted; s->nmiWanted = c->nmiWanted;
}

static void snap_restore(const Snap *s, Snes *snes) {
    memcpy(snes->ram, s->ram, sizeof(s->ram));
    Cpu *c = snes->cpu;
    c->a = s->a; c->x = s->x; c->y = s->y;
    c->sp = s->sp; c->pc = s->pc; c->dp = s->dp;
    c->k = s->k; c->db = s->db;
    c->c = s->c; c->z = s->z; c->v = s->v; c->n = s->n;
    c->i = s->i; c->d = s->d; c->xf = s->xf; c->mf = s->mf; c->e = s->e;
    c->waiting = s->waiting; c->stopped = s->stopped;
    c->irqWanted = s->irqWanted; c->nmiWanted = s->nmiWanted;
}

// ---------------------------------------------------------------------------
// C side: CalcHits reimplemented.
//
// Reference (damage.asm @c987):
//   CalcHits:
//   @c987:  stz     $38fd       ; clear number of hits
//           lda     $38fb
//           beq     @c99e       ; return if no base hits
//           tay
//   @c990:  jsr     Rand99
//           cmp     $38fa       ; check vs. hit rate
//           bcs     @c99b       ; if Rand99 >= rate, skip
//           inc     $38fd       ; increment number of hits
//   @c99b:  dey
//           bne     @c990
//   @c99e:  rts
//
// Semantics: `cmp` then `bcs` ⇔ "if A >= mem branch" → our `r < rate`
// inverts the condition (bcs does NOT branch when carry is clear, i.e.
// when A < mem). So: if Rand99 < rate, we increment.
// ---------------------------------------------------------------------------

static uint8_t rand99_emu(Snes *snes) {
    // Snapshot the minimal CPU regs so the state between draws stays clean.
    Cpu *c = snes->cpu;
    uint16_t saved_a = c->a, saved_x = c->x, saved_y = c->y;
    uint16_t saved_sp = c->sp, saved_pc = c->pc, saved_dp = c->dp;
    uint8_t saved_k = c->k, saved_db = c->db;
    bool saved_mf = c->mf, saved_xf = c->xf;

    // Setup for Rand99: A/X 8-bit mode, DB=0x7E (Rand99 does `lda $1900,x`
    // which must point into WRAM).
    c->dp = 0;
    c->db = 0x7E;
    c->mf = true;
    c->xf = true;

    run_emulated_func(snes, RAND99_ADDR_24);

    uint8_t result = (uint8_t)(c->a & 0xFF);

    c->a = saved_a; c->x = saved_x; c->y = saved_y;
    c->sp = saved_sp; c->pc = saved_pc; c->dp = saved_dp;
    c->k = saved_k; c->db = saved_db;
    c->mf = saved_mf; c->xf = saved_xf;

    return result;
}

static void CalcHits_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[RAM_NHITS_OUT] = 0;
    uint8_t base = ram[RAM_BASE_HITS];
    if (base == 0) return;
    uint8_t rate = ram[RAM_HIT_RATE];
    for (uint8_t y = base; y > 0; y--) {
        uint8_t r = rand99_emu(snes);
        if (r < rate) {
            ram[RAM_NHITS_OUT]++;
        }
    }
}

// ---------------------------------------------------------------------------
// ASM side: a thin wrapper that sets DB/DP/M/X and calls CalcHits.
// ---------------------------------------------------------------------------

static void CalcHits_asm(Snes *snes) {
    Cpu *c = snes->cpu;
    c->dp = 0;
    c->db = 0x7E;
    c->mf = true;
    c->xf = true;
    run_emulated_func(snes, CALCHITS_ADDR_24);
}

// ---------------------------------------------------------------------------
// Host PRNG — xorshift32 used to generate trial inputs.
// ---------------------------------------------------------------------------

static uint32_t host_rng_state = 0x12345678u;
static uint32_t host_rng(void) {
    uint32_t x = host_rng_state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    host_rng_state = x;
    return x;
}
static uint8_t host_rng_byte(void) { return (uint8_t)(host_rng() & 0xFF); }

// ---------------------------------------------------------------------------
// Trial
// ---------------------------------------------------------------------------

typedef struct {
    uint8_t rng_index;
    uint8_t base_hits;
    uint8_t hit_rate;
    uint8_t rng_table[256];
} Trial;

static int run_one_trial(Snes *snes, const Trial *t, Snap *baseline,
                         int trial_id, bool verbose) {
    // Prepare the baseline: restore the "stable boot" state and write inputs.
    snap_restore(baseline, snes);
    memcpy(snes->ram + RAM_RNG_TABLE, t->rng_table, 256);
    snes->ram[RAM_RNG_INDEX] = t->rng_index;
    snes->ram[RAM_HIT_RATE] = t->hit_rate;
    snes->ram[RAM_BASE_HITS] = t->base_hits;
    snes->ram[RAM_NHITS_OUT] = 0xAA;  // sentinel

    // Pre-asm snapshot (so we can replay the C side with the exact same state).
    Snap pre;
    snap_take(&pre, snes);

    // RUN ASM
    CalcHits_asm(snes);
    uint8_t out_asm = snes->ram[RAM_NHITS_OUT];

    // Restore pre-asm, run C
    snap_restore(&pre, snes);
    CalcHits_c(snes);
    uint8_t out_c = snes->ram[RAM_NHITS_OUT];

    bool ok = (out_asm == out_c);
    if (!ok || verbose) {
        printf("trial %4d : idx=%3u base=%3u rate=%3u  asm=%3u c=%3u  %s\n",
               trial_id, t->rng_index, t->base_hits, t->hit_rate,
               out_asm, out_c, ok ? "OK" : "FAIL");
    }
    return ok ? 0 : 1;
}

// ---------------------------------------------------------------------------

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
    if (!snes_loadRom(snes, rom, rom_len)) {
        fprintf(stderr, "snes_loadRom failed\n"); return 3;
    }
    snes_reset(snes, true);
    for (int i = 0; i < 60; i++) snes_runFrame(snes);

    snes->cpu->i = true;
    snes->cpu->nmiWanted = false;
    snes->cpu->irqWanted = false;

    // Baseline = stable CPU+RAM state after 60 boot frames.
    Snap baseline;
    snap_take(&baseline, snes);

    fprintf(stderr, "comparing CalcHits asm vs C : %d trials\n", n_trials);

    int fails = 0;
    Trial t;
    for (int i = 0; i < n_trials; i++) {
        t.rng_index = host_rng_byte();
        t.base_hits = host_rng_byte();
        t.hit_rate = (uint8_t)(host_rng() % 100);  // [0, 99] = in-game domain
        for (int j = 0; j < 256; j++) t.rng_table[j] = host_rng_byte();
        fails += run_one_trial(snes, &t, &baseline, i, /*verbose*/false);
    }

    printf("\n=== summary ===\ntrials : %d\nfails  : %d\n", n_trials, fails);
    free(rom);
    return fails == 0 ? 0 : 1;
}
