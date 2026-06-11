// Spike M6 — ADR-003 test on ApplyDmg (90 instr, explicit longa).
//
// Step 1 (this code) — Delegation validation:
//   * Minimal combat setup: 13 target slots, random HPs/flags.
//   * Run ApplyDmg via run_emulated_func.
//   * Verify: ApplyDmg executes (returns to the magic PC) and modifies HPs
//     the way we expect for "damage" non-miss targets.
//
// Step 2 — Translation feasibility analysis (offline, manual):
//   count potential B-hidden chains, branches, etc.
//
// The goal here is NOT to compare asm vs C (no C is produced), but to
// confirm that the "delegate" strategy works for routines that combine
// explicit `longa` with an outer loop. If it does, ADR-003 stands:
// "delegate by default for instr_count > 50" — a future translation
// remains possible but is deferred.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes.h"
#include "cpu.h"

#define APPLY_DMG_ADDR_24 0x03CA6Eu

#define N_TARGETS 13
#define TARGET_STRIDE 0x80          // X += $80 between targets ($cb22: ADC #$0080)

#define RAM_TARGET_PRESENT 0x3540   // 1 byte per target (Y indexed)
#define RAM_ATTACK_FLAGS   0x34D4   // 2 bytes per target (Y indexed)
#define RAM_TARGET_HP_LO   0x2007   // 2 bytes per target (X indexed, stride 0x80)
#define RAM_TARGET_HP_MAX  0x2009   // same, max HP
#define RAM_STATUS_DEAD    0x338E
#define RAM_STATUS_CRIT    0x3391
#define RAM_DEAD_COUNT     0x3907   // incremented by ApplyDmg when HP reaches 0
#define RAM_SHOW_ZERO_DMG  0x355B   // "show zero damage" display flag
#define RAM_A9_INDEX       0xA9

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
    long max_ops = 500000;  // boucle 13 cibles = ~13×30 = 400 opcodes max
    while (cpu->sp != sp_save && max_ops-- > 0) {
        cpu_runOpcode(cpu);
        if (cpu->waiting || cpu->stopped) break;
    }
}

static inline uint16_t read16(const uint8_t *ram, int addr) {
    return (uint16_t)(ram[addr] | (ram[addr + 1] << 8));
}
static inline void write16(uint8_t *ram, int addr, uint16_t v) {
    ram[addr] = v & 0xFF; ram[addr + 1] = (v >> 8) & 0xFF;
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
// Delegation wrapper for ApplyDmg
// ---------------------------------------------------------------------------

static void ApplyDmg_emu(Snes *snes) {
    Cpu *c = snes->cpu;
    c->dp = 0;
    c->db = 0x7E;
    // ApplyDmg commence par `longa / clr_ax / stx $a9 / tay`.
    // longa fait `rep #$20` → A passe 16-bit. mais clr_ax = tdc/tax,
    // so the initial mf doesn't really matter.
    // X 16-bit : ApplyDmg utilise X 16-bit (stx $a9 word).
    c->mf = true;   // will be changed by `longa`
    c->xf = false;  // X 16-bit
    c->a = 0; c->x = 0; c->y = 0;
    c->z = true; c->n = false;
    run_emulated_func(snes, APPLY_DMG_ADDR_24);
}

// ---------------------------------------------------------------------------
// Test : setup combat minimal, run ApplyDmg, observer
// ---------------------------------------------------------------------------

typedef struct {
    bool present;
    uint16_t hp_current;
    uint16_t hp_max;
    uint16_t attack_flag;  // bit 15 = damage(0)/heal(1), bit 14 = miss
} TargetSetup;

static void setup_targets(Snes *snes, TargetSetup *targets) {
    uint8_t *ram = snes->ram;
    // clear flags globaux
    ram[RAM_DEAD_COUNT] = 0;
    ram[RAM_SHOW_ZERO_DMG] = 0;

    for (int i = 0; i < N_TARGETS; i++) {
        int y_off = i * 2;
        int x_off = i * TARGET_STRIDE;

        // target presence : 1 byte at $3540+(i*2)
        ram[RAM_TARGET_PRESENT + y_off] = targets[i].present ? 1 : 0;
        // attack flag : 2 bytes at $34D4+(i*2)
        write16(ram, RAM_ATTACK_FLAGS + y_off, targets[i].attack_flag);
        // hp current : 2 bytes at $2007+(i*0x80)
        write16(ram, RAM_TARGET_HP_LO + x_off, targets[i].hp_current);
        // hp max : 2 bytes at $2009+(i*0x80)
        write16(ram, RAM_TARGET_HP_MAX + x_off, targets[i].hp_max);
        // clear status bytes
        ram[RAM_STATUS_DEAD + x_off] = 0;
        ram[RAM_STATUS_CRIT + x_off] = 0;
        ram[0x2006 + x_off] = 0;  // adjacent critical bit
    }
}

static void dump_targets(const Snes *snes, const char *label) {
    const uint8_t *ram = snes->ram;
    printf("--- %s ---\n", label);
    printf("dead_count=%u\n", ram[RAM_DEAD_COUNT]);
    for (int i = 0; i < N_TARGETS; i++) {
        int x_off = i * TARGET_STRIDE;
        int y_off = i * 2;
        uint16_t af = read16(ram, RAM_ATTACK_FLAGS + y_off);
        uint16_t hp = read16(ram, RAM_TARGET_HP_LO + x_off);
        uint8_t status = ram[RAM_STATUS_DEAD + x_off];
        printf("  T%2d: present=%u  attack_flag=%04X  HP=%5u  status_dead=%02X\n",
               i, ram[RAM_TARGET_PRESENT + y_off], af, hp, status);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s <rom.sfc>\n", argv[0]); return 1; }
    size_t rom_len = 0;
    uint8_t *rom = read_file(argv[1], &rom_len);
    if (!rom) return 2;

    Snes *snes = snes_init();
    if (!snes_loadRom(snes, rom, rom_len)) return 3;
    snes_reset(snes, true);
    for (int i = 0; i < 60; i++) snes_runFrame(snes);
    snes->cpu->i = true; snes->cpu->nmiWanted = false; snes->cpu->irqWanted = false;

    Snap baseline;
    snap_take(&baseline, snes);

    // --- Test 1: 3 targets present, attack flag = damage (bit 15 = 0), damage value 100
    //             Aucune en miss (bit 14=0). HP courant = 500 chaque.
    TargetSetup targets[N_TARGETS] = {0};
    targets[0] = (TargetSetup){.present=true, .hp_current=500, .hp_max=999, .attack_flag=0x0064};  // damage=100
    targets[1] = (TargetSetup){.present=true, .hp_current=300, .hp_max=999, .attack_flag=0x0064};  // damage=100
    targets[2] = (TargetSetup){.present=true, .hp_current=50,  .hp_max=999, .attack_flag=0x0064};  // damage=100, devrait tuer
    // autres : absents

    snap_restore(&baseline, snes);
    setup_targets(snes, targets);

    dump_targets(snes, "PRE-RUN");

    Snap pre; snap_take(&pre, snes);
    ApplyDmg_emu(snes);

    Cpu *c = snes->cpu;
    printf("\npost-asm: PC=%02X:%04X SP=%04X mf=%d xf=%d (expect PC=$03:1235 SP=%04X)\n",
           c->k, c->pc, c->sp, c->mf, c->xf, pre.sp);

    dump_targets(snes, "POST-RUN");

    free(rom);
    return 0;
}
