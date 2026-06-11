// Spike M4 — Comparison asm vs C de GetDmgPtr.
// Translation produced by simulating the reverser-agent prompt template
// (Phase 3.5 dry-run, agent role played by Claude in chat).
//
// ASM source @ $03:CA62 (battle/damage.asm) :
//   GetDmgPtr:
//   sta $a9 / bpl @ca6b / and #$7f / clc / adc #$05
//   @ca6b: asl / tax / rts
//
// Output: cpu->x = offset, ram[$A9] = input

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes.h"
#include "cpu.h"

#define GET_DMG_PTR_ADDR_24 0x03CA62u

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
// C translation produced by the prompt template
// ---------------------------------------------------------------------------

static uint16_t GetDmgPtr_c(Snes *snes, uint8_t target_id) {
    uint8_t *ram = snes->ram;
    ram[0xA9] = target_id;
    uint8_t a;
    if ((target_id & 0x80) == 0) {
        a = target_id;                   // character
    } else {
        a = (target_id & 0x7F) + 5;      // enemy
    }
    // PIÈGE : en mode A 8-bit, `asl` tronque à 8 bits. En C, `a << 1`
    // promote à int et garde le bit 8. Forcer la troncature.
    return (uint16_t)(uint8_t)(a << 1);
}

// ---------------------------------------------------------------------------
// ASM wrapper
// ---------------------------------------------------------------------------

static uint16_t GetDmgPtr_asm(Snes *snes, uint8_t target_id) {
    Cpu *c = snes->cpu;
    c->dp = 0;
    c->db = 0x7E;
    c->mf = true;
    c->xf = true;          // X 8-bit pour clean (tax met X_hi=0 en 8-bit X)
    c->a = target_id;
    c->x = 0; c->y = 0;
    // Pitfall 2 : `bpl` consulte N qui n'est pas set par `sta`. Simuler N.
    c->n = (target_id & 0x80) != 0;
    c->z = (target_id == 0);  // par cohérence (pas utilisé par bpl mais safe)
    run_emulated_func(snes, GET_DMG_PTR_ADDR_24);
    return (uint16_t)(c->x & 0xFF);  // X 8-bit → result low byte only
}

// ---------------------------------------------------------------------------
// Snapshot / restore (réutilise pattern M2/M3)
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

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s <rom.sfc>\n", argv[0]); return 1; }
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

    fprintf(stderr, "exhaustive: GetDmgPtr asm vs C, all 256 inputs\n");

    int fails = 0;
    for (int input = 0; input < 256; input++) {
        snap_restore(&baseline, snes);
        uint16_t out_asm = GetDmgPtr_asm(snes, (uint8_t)input);
        uint8_t ram_a9_asm = snes->ram[0xA9];

        snap_restore(&baseline, snes);
        uint16_t out_c = GetDmgPtr_c(snes, (uint8_t)input);
        uint8_t ram_a9_c = snes->ram[0xA9];

        bool ok = (out_asm == out_c) && (ram_a9_asm == ram_a9_c);
        if (!ok) {
            printf("input=%3d (0x%02X)  asm: X=%5u $A9=%02X  c: X=%5u $A9=%02X  FAIL\n",
                   input, input, out_asm, ram_a9_asm, out_c, ram_a9_c);
            fails++;
        }
    }

    printf("\n=== summary === inputs tested: 256, fails: %d\n", fails);
    free(rom);
    return fails == 0 ? 0 : 1;
}
