// Spike M5 — Test du template sur CalcDmg (60 instr asm), scale test.
//
// 7 blocs identifiés dans damage.asm @c99f :
//   1. variance = base/2, clamp 0xFF si high byte nonzero
//   2. variance = RandXA(0, variance) ; damage = variance + base (via Add16)
//   3. apply elemental multiplier (ApplyDmgMult)
//   4. apply creature type multiplier (ApplyDmgMult)
//   5. crit bonus (16-bit add)
//   6. subtract defense (16-bit sbc, clamp to 0)
//   7. atk_mult via Mult16, saturate to 0xFFFF if high nonzero
//   8. def_mult via Div16, cap 9999, min 1
//
// Inputs en WRAM :
//   $3902 base attack (16-bit)
//   $38FA hit_rate, $38FB base_hits (non utilisés ici)
//   $38FC atk_mult (8-bit)
//   $38FE elemental mult, $38FF creature type mult (8-bit each)
//   $3900 crit flag (0=no crit), $3901 crit bonus (8-bit)
//   $3904-$3905 defense (16-bit)
//   $3906 def_mult (8-bit)
//   $1900..$19FF RNG table
//   $97 rng_index
//
// Output : $A4-$A5 = damage 16-bit, capped to [1, 9999=0x270F]

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes.h"
#include "cpu.h"

#define CALC_DMG_ADDR_24       0x03C99Fu
#define APPLY_DMG_MULT_ADDR_24 0x03CA41u
#define MULT16_ADDR_24         0x0383B9u
#define DIV16_ADDR_24          0x038407u
#define ADD16_ADDR_24          0x0384E3u
#define RAND_XA_ADDR_24        0x038379u

// ---------------------------------------------------------------------------
// Boilerplate (factoriser plus tard)
// ---------------------------------------------------------------------------

static uint8_t *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *buf = malloc(sz);
    if (fread(buf, 1, sz, f) != (size_t)sz) { free(buf); fclose(f); return NULL; }
    fclose(f); *out_len = sz; return buf;
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
    long max_ops = 200000;
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

static inline uint16_t read16(const uint8_t *ram, int addr) {
    return (uint16_t)(ram[addr] | (ram[addr + 1] << 8));
}
static inline void write16(uint8_t *ram, int addr, uint16_t v) {
    ram[addr] = v & 0xFF; ram[addr + 1] = (v >> 8) & 0xFF;
}

// ---------------------------------------------------------------------------
// Helpers *_emu : delegate to asm via RunEmulatedFunc
// Convention : longa/shorta géré par les routines elles-mêmes ; on les appelle
// en setant DB=$7E DP=0, mode A 8-bit (par défaut battle) — elles passent en
// 16-bit par leur premier `longa` au besoin.
// ---------------------------------------------------------------------------

static void call_emu(Snes *snes, uint32_t addr, bool mf_in, bool xf_in,
                     uint16_t a_in, uint16_t x_in, uint16_t y_in,
                     bool z_in, bool n_in) {
    Cpu *c = snes->cpu;
    uint16_t saved_a=c->a, saved_x=c->x, saved_y=c->y, saved_sp=c->sp;
    uint16_t saved_pc=c->pc, saved_dp=c->dp;
    uint8_t saved_k=c->k, saved_db=c->db;
    bool saved_mf=c->mf, saved_xf=c->xf, saved_z=c->z, saved_n=c->n;
    c->dp=0; c->db=0x7E;
    c->mf=mf_in; c->xf=xf_in;
    c->a=a_in; c->x=x_in; c->y=y_in;
    c->z=z_in; c->n=n_in;
    run_emulated_func(snes, addr);
    // restore modes — content of RAM is intentionally preserved
    c->a=saved_a; c->x=saved_x; c->y=saved_y;
    c->sp=saved_sp; c->pc=saved_pc; c->dp=saved_dp;
    c->k=saved_k; c->db=saved_db;
    c->mf=saved_mf; c->xf=saved_xf; c->z=saved_z; c->n=saved_n;
}

// RandXA : in A=max, X=min ; out A=rand[min..max]
// Routine asm fait `shorti` au début → X devient 8-bit
// Mode A à l'entrée : 16-bit selon CalcDmg context
static uint16_t rand_xa_emu(Snes *snes, uint16_t a_max, uint8_t x_min) {
    Cpu *c = snes->cpu;
    uint16_t saved_a=c->a, saved_x=c->x, saved_y=c->y, saved_sp=c->sp;
    uint16_t saved_pc=c->pc, saved_dp=c->dp;
    uint8_t saved_k=c->k, saved_db=c->db;
    bool saved_mf=c->mf, saved_xf=c->xf, saved_z=c->z, saved_n=c->n;
    c->dp=0; c->db=0x7E;
    c->mf=false;  // A 16-bit (héritage longa)
    c->xf=false;  // X 16-bit (sera shorti par RandXA)
    c->a=a_max; c->x=x_min; c->y=0;
    // Z et N selon a_max (BEQ check in RandXA tests A==0)
    c->z = (a_max == 0); c->n = (a_max & 0x8000) != 0;
    run_emulated_func(snes, RAND_XA_ADDR_24);
    uint16_t result = c->a;
    c->x=saved_x; c->y=saved_y;
    c->sp=saved_sp; c->pc=saved_pc; c->dp=saved_dp;
    c->k=saved_k; c->db=saved_db;
    c->mf=saved_mf; c->xf=saved_xf; c->z=saved_z; c->n=saved_n;
    c->a=saved_a;  // restore but we already captured result
    return result;
}

static void add16_emu(Snes *snes)  { call_emu(snes, ADD16_ADDR_24,  true, false, 0, 0, 0, false, false); }
static void mult16_emu(Snes *snes) { call_emu(snes, MULT16_ADDR_24, true, false, 0, 0, 0, false, false); }
static void div16_emu(Snes *snes)  { call_emu(snes, DIV16_ADDR_24,  true, false, 0, 0, 0, false, false); }

static void apply_dmg_mult_emu(Snes *snes, uint8_t mult) {
    call_emu(snes, APPLY_DMG_MULT_ADDR_24, true, false, mult, 0, 0, (mult==0), (mult&0x80)!=0);
}

// ---------------------------------------------------------------------------
// Côté ASM
// ---------------------------------------------------------------------------

static uint16_t CalcDmg_asm(Snes *snes) {
    Cpu *c = snes->cpu;
    c->dp = 0; c->db = 0x7E;
    // Pitfall 8 CONFIRMÉ : CalcDmg n'a pas de longa/shorta explicite mais
    // attend mode A 8-bit (hérité convention battle). En mode 16-bit, le
    // `lsr $3957` shift le WORD $3957-$3958 et corrompt $3958 (le backup de
    // base pour Add16). Diagnostic A1 a montré $3958=0115 vs 012A attendu.
    c->mf = true;   // A 8-bit (convention battle)
    c->xf = false;  // X 16-bit
    c->a = 0; c->x = 0; c->y = 0;
    c->z = true; c->n = false;
    run_emulated_func(snes, CALC_DMG_ADDR_24);
    return read16(snes->ram, 0xA4);
}

// ---------------------------------------------------------------------------
// Côté C — traduction selon le template
// ---------------------------------------------------------------------------

// Variante "déléguer tout" — utilisée pour valider l'hypothèse "fonction
// de composition non traduisible sans tracker B-caché".
// Si cette version PASSE 1000/1000, l'archi shadow-exec marche mais CalcDmg
// doit être traitée comme "boîte noire émulée" plutôt que traduite.
static uint16_t CalcDmg_c_delegated(Snes *snes) {
    Cpu *c = snes->cpu;
    c->dp = 0; c->db = 0x7E;
    c->mf = true; c->xf = false;
    c->a = 0; c->x = 0; c->y = 0;
    c->z = true; c->n = false;
    run_emulated_func(snes, CALC_DMG_ADDR_24);
    return read16(snes->ram, 0xA4);
}

static uint16_t CalcDmg_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Bloc 1 : variance = base/2, clamp to 0x00FF si high nonzero
    uint16_t base = read16(ram, 0x3902);
    write16(ram, 0x3956, base);   // current dmg / variance
    write16(ram, 0x3958, base);   // backup pour Add16
    uint16_t variance = base >> 1;
    write16(ram, 0x3956, variance);
    // lda $3957 / beq → test high byte de variance (= variance >> 8)
    // Note : lda $3957 charge 1 octet en mode A 8-bit, mais on est en A 16-bit
    // ici (héritage longa) → lda $3957 charge $3957 et $3958. Donc le test
    // beq porte sur le mot ($3957 + $3958*256).
    // ATTENTION : ici l'asm fait potentiellement quelque chose de subtil.
    // Inspection : @c9a4 vient juste avant lsr/ror, donc on est ENCORE en mode
    // hérité du début. Le début de CalcDmg ne contient PAS de shorta/longa
    // explicite, le mode est celui du caller.
    // Pour l'instant je suppose A 16-bit (à valider par test).
    // En mode A 16-bit : lda $3957 charge ($3957=v_hi) en A_lo et ($3958=base_lo)
    // en A_hi. C'est BIZARRE — peut-être que le code attend que $3958 ait été
    // backed up à `base` initial, et que $3957 (high de variance) soit nonzero
    // signifie "variance dépasse 0xFF".
    // Approche pragmatique : tester si v_hi != 0 (i.e. variance > 0xFF) et
    // clamper à 0x00FF.
    if ((variance >> 8) != 0) {
        write16(ram, 0x3956, 0x00FF);
    }

    // Bloc 2 : variance = RandXA(0, variance) ; damage = variance + base
    uint16_t v = read16(ram, 0x3956);
    uint16_t rand_v = rand_xa_emu(snes, v, 0);
    write16(ram, 0x3956, rand_v);
    add16_emu(snes);              // $395a = $3956 + $3958
    uint16_t damage = read16(ram, 0x395A);
    write16(ram, 0xA4, damage);

    // Bloc 3-4 : apply elemental + creature type multipliers
    apply_dmg_mult_emu(snes, ram[0x38FE]);  // elemental
    apply_dmg_mult_emu(snes, ram[0x38FF]);  // creature type

    // Bloc 5 : crit bonus (16-bit add)
    uint8_t crit = ram[0x3900];
    if (crit != 0) {
        uint16_t d = read16(ram, 0xA4);
        d += ram[0x3901];
        write16(ram, 0xA4, d);
    }

    // Bloc 6 : subtract defense (16-bit sbc, clamp to 0 si underflow)
    {
        uint16_t d = read16(ram, 0xA4);
        uint16_t def = read16(ram, 0x3904);
        if (d >= def) {
            write16(ram, 0xA4, d - def);
        } else {
            write16(ram, 0xA4, 0);
        }
    }

    // Blocs 7+8 : DÉLÉGUÉS à l'asm (Pitfall 9 : B caché non trackable en C
    // sans tracker explicite). Sous-portion @ca01-@ca40 (mult atk_mult,
    // saturate, div def_mult, cap 9999, min 1).
    //
    // Pour passer 1000/1000 PASS, il faudra revenir traduire 7+8 en C
    // avec tracking de B. Pour le spike, la délégation prouve que les
    // blocs 1-6 sont corrects.
    {
        Cpu *c = snes->cpu;
        c->dp = 0; c->db = 0x7E;
        c->mf = true; c->xf = false;
        c->a = 0; c->x = 0; c->y = 0;
        run_emulated_func(snes, 0x03CA01u);
    }

    return read16(ram, 0xA4);
}

// ---------------------------------------------------------------------------
// Trial
// ---------------------------------------------------------------------------

static uint32_t host_rng_state = 0xCAFEBABEu;
static uint32_t host_rng(void) {
    uint32_t x = host_rng_state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    host_rng_state = x; return x;
}

static int run_trial(Snes *snes, Snap *baseline, int trial_id, bool verbose) {
    snap_restore(baseline, snes);
    uint8_t *ram = snes->ram;

    // Inputs aléatoires
    write16(ram, 0x3902, (uint16_t)(host_rng() & 0x03FF));  // base attack 0..1023
    ram[0x38FC] = 1 + (uint8_t)(host_rng() % 8);            // atk_mult 1..8 (jamais 0 sinon mult par 0)
    ram[0x38FE] = (uint8_t)(host_rng() & 0xFF);             // elemental
    ram[0x38FF] = (uint8_t)(host_rng() & 0xFF);             // creature
    ram[0x3900] = (host_rng() & 1) ? 1 : 0;                  // crit ?
    ram[0x3901] = (uint8_t)(host_rng() & 0xFF);             // crit bonus
    write16(ram, 0x3904, (uint16_t)(host_rng() & 0x01FF));   // defense 0..511
    ram[0x3906] = 1 + (uint8_t)(host_rng() % 8);            // def_mult 1..8 (jamais 0)
    ram[0x97] = (uint8_t)(host_rng() & 0xFF);                // rng_index
    // RNG table
    for (int i = 0; i < 256; i++) ram[0x1900 + i] = (uint8_t)(host_rng() & 0xFF);

    Snap pre;
    snap_take(&pre, snes);

    uint16_t out_asm = CalcDmg_asm(snes);

    // === DIAG A1 — état post-asm pour le trial 0 uniquement ===
    if (trial_id == 0) {
        Cpu *c = snes->cpu;
        fprintf(stderr, "DIAG A1 post-asm: PC=%02X:%04X SP=%04X mf=%d xf=%d\n",
                c->k, c->pc, c->sp, c->mf, c->xf);
        fprintf(stderr, "  $A4=%02X $A5=%02X (16-bit dmg post-asm)\n",
                snes->ram[0xA4], snes->ram[0xA5]);
        fprintf(stderr, "  $3956=%02X%02X $3958=%02X%02X $395a=%02X%02X\n",
                snes->ram[0x3957], snes->ram[0x3956],
                snes->ram[0x3959], snes->ram[0x3958],
                snes->ram[0x395B], snes->ram[0x395A]);
    }

    snap_restore(&pre, snes);
    // Use "delegated" variant to test hypothesis. Switch to CalcDmg_c for
    // partial-translation test.
    uint16_t out_c = CalcDmg_c_delegated(snes);
    (void)CalcDmg_c;  // silence unused warning

    bool ok = (out_asm == out_c);
    if (!ok || verbose) {
        printf("trial %4d : base=%5u def=%5u atk_mul=%3u def_mul=%3u elem=%3u "
               "crit=%u crit_b=%3u  asm=%5u c=%5u  %s\n",
               trial_id,
               read16(pre.ram, 0x3902), read16(pre.ram, 0x3904),
               pre.ram[0x38FC], pre.ram[0x3906], pre.ram[0x38FE],
               pre.ram[0x3900], pre.ram[0x3901],
               out_asm, out_c, ok ? "OK" : "FAIL");
    }
    return ok ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s <rom.sfc> [n_trials]\n", argv[0]); return 1; }
    int n_trials = (argc >= 3) ? atoi(argv[2]) : 100;

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

    // === DIAG A1 ===
    fprintf(stderr, "DIAG A1: baseline[$A4]=%02X [$A5]=%02X (post-boot residual at damage slot)\n",
            baseline.ram[0xA4], baseline.ram[0xA5]);
    fprintf(stderr, "DIAG A1: baseline CPU: PC=%02X:%04X SP=%04X DP=%04X DB=%02X mf=%d xf=%d e=%d\n",
            baseline.k, baseline.pc, baseline.sp, baseline.dp, baseline.db,
            baseline.mf, baseline.xf, baseline.e);

    fprintf(stderr, "CalcDmg asm vs C : %d trials\n", n_trials);

    int fails = 0;
    for (int i = 0; i < n_trials; i++) {
        fails += run_trial(snes, &baseline, i, /*verbose*/false);
    }

    printf("\n=== summary === trials: %d, fails: %d\n", n_trials, fails);
    free(rom);
    return fails == 0 ? 0 : 1;
}
