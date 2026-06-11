// Spike M2 — comparison asm vs C de CalcHits, avec shadow-exec partiel.
//
// Architecture :
//   - 1 seule instance LakeSnes
//   - Côté ASM : on positionne le PC sur CalcHits @ $03:C987 et on laisse
//     l'asm tourner (qui appelle Rand99 lui-même via JSR)
//   - Côté C : CalcHits_c(snes) reproduit la logique de damage.asm en C, et
//     pour chaque tirage Rand99 il fait RunEmulatedFunc($03:858B) qui exécute
//     l'asm Rand99 et retourne dans cpu->a. C'est EXACTEMENT le pattern
//     zelda3 "fonction traduite délègue à l'asm pour les sous-routines non
//     traduites".
//
//   - Pour chaque trial : snapshot RAM + regs → run asm → record output → restore
//     → run C → record output → compare byte-equal sur $38FD
//
// Si 1000/1000 trials donnent CalcHits_asm == CalcHits_c, l'archi
// shadow-exec snesrev est définitivement validée sur FF4.
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
// On utilise un buffer alloué une fois.
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
// Côté C : CalcHits réimplémenté.
//
// Référence (damage.asm @c987) :
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
// Sémantique : `cmp` puis `bcs` ⇔ if A >= mem branche → notre `r < rate`
// inverse la condition (puisque bcs ne branche PAS quand carry clear, c.à.d
// quand A < mem). Donc : si Rand99 < rate, on incrémente.
// ---------------------------------------------------------------------------

static uint8_t rand99_emu(Snes *snes) {
    // Snapshot CPU regs minimaux pour ne pas pourrir l'état entre 2 tirages
    Cpu *c = snes->cpu;
    uint16_t saved_a = c->a, saved_x = c->x, saved_y = c->y;
    uint16_t saved_sp = c->sp, saved_pc = c->pc, saved_dp = c->dp;
    uint8_t saved_k = c->k, saved_db = c->db;
    bool saved_mf = c->mf, saved_xf = c->xf;

    // Setup pour Rand99 : mode A/X 8-bit, DB=0x7E (Rand99 fait lda $1900,x
    // qui doit pointer en WRAM)
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
// Côté ASM : juste un wrapper qui set DB/DP/M/X et appelle CalcHits.
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
// Host PRNG — xorshift32 pour générer les inputs des trials
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
    // Préparer le baseline : restore l'état "boot stable" + inscrire les inputs
    snap_restore(baseline, snes);
    memcpy(snes->ram + RAM_RNG_TABLE, t->rng_table, 256);
    snes->ram[RAM_RNG_INDEX] = t->rng_index;
    snes->ram[RAM_HIT_RATE] = t->hit_rate;
    snes->ram[RAM_BASE_HITS] = t->base_hits;
    snes->ram[RAM_NHITS_OUT] = 0xAA;  // sentinelle

    // Snapshot pré-asm (pour pouvoir rejouer le C avec exactement le même état)
    Snap pre;
    snap_take(&pre, snes);

    // RUN ASM
    CalcHits_asm(snes);
    uint8_t out_asm = snes->ram[RAM_NHITS_OUT];

    // Restore pré-asm, run C
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

    // Baseline = état CPU+RAM stable après 60 frames boot
    Snap baseline;
    snap_take(&baseline, snes);

    fprintf(stderr, "comparing CalcHits asm vs C : %d trials\n", n_trials);

    int fails = 0;
    Trial t;
    for (int i = 0; i < n_trials; i++) {
        t.rng_index = host_rng_byte();
        t.base_hits = host_rng_byte();
        t.hit_rate = (uint8_t)(host_rng() % 100);  // dans [0, 99] = domaine de jeu
        for (int j = 0; j < 256; j++) t.rng_table[j] = host_rng_byte();
        fails += run_one_trial(snes, &t, &baseline, i, /*verbose*/false);
    }

    printf("\n=== summary ===\ntrials : %d\nfails  : %d\n", n_trials, fails);
    free(rom);
    return fails == 0 ? 0 : 1;
}
