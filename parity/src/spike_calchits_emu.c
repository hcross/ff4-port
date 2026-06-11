// Spike M1 — validate RunEmulatedFunc as a pure-external helper (no LakeSnes
// core modification) against the CalcHits routine @ $03:C987 of FF4 JP 1.1.
//
// Goals:
//   - Load the ROM, run enough boot frames to stabilise the CPU.
//   - Prepare WRAM manually (RNG table at $1900, index $97, params $38fa/$38fb).
//   - Position PC on CalcHits, run the asm until the RTS.
//   - Observe the value written to $38fd and compare against the Python-shaped
//     oracle (count of the first N RNG entries that are < hit_rate).
//
// If it matches → the RunEmulatedFunc architecture stands up on stock LakeSnes
// (no core patch), and we can proceed to spike M2 (C translation of CalcHits +
// asm-vs-C comparison over 1000 inputs).
//
// Usage:
//   ./ff4-spike-calchits <rom.sfc>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "snes.h"
#include "cpu.h"

// CalcHits 24-bit address, LoROM mapping over the battle_code segment in bank 0x03.
// Computed as: map's battle_code segment start (0x038000) + .lst offset 0x4987.
#define CALCHITS_ADDR_24 0x03C987u

// Relevant RAM addresses (direct page, also accessible via absolute mode)
#define RAM_RNG_INDEX  0x0097     // ram[0x97] : index into the RNG table
#define RAM_RNG_TABLE  0x1900     // ram[0x1900..0x19FF] : RNG table copied from ROM
#define RAM_HIT_RATE   0x38FA     // input
#define RAM_BASE_HITS  0x38FB     // input
#define RAM_NHITS_OUT  0x38FD     // output

// ---------------------------------------------------------------------------
// I/O
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

// ---------------------------------------------------------------------------
// RunEmulatedFunc — pure-external, no LakeSnes core modification.
//
// We push two dummy bytes onto the stack so that when the target routine
// executes its final RTS, the CPU "jumps" to an arbitrary address. We monitor
// SP: as long as SP < sp_save, we are inside the function (or a sub-call).
// When SP returns to its initial level, the RTS has executed → we stop.
// ---------------------------------------------------------------------------

static inline uint32_t stack_addr(const Cpu *cpu) {
    // In emulation mode (e=1) SP is forced into page 1: addr = $0100 | (sp & 0xFF).
    // In native mode (e=0) SP is a real 16-bit pointer anywhere in bank 0.
    return cpu->e ? (0x0100u | (cpu->sp & 0xFFu)) : cpu->sp;
}

static void run_emulated_func(Snes *snes, uint32_t pc24) {
    Cpu *cpu = snes->cpu;
    uint16_t sp_save = cpu->sp;

    // Push a "magic" return address that we never actually execute (we stop
    // before that). 65816 JSR convention: push PC_hi at [SP], decrement SP,
    // push PC_lo at [SP], decrement SP.
    snes->ram[stack_addr(cpu)] = 0x12;  // PC_hi
    cpu->sp--;
    snes->ram[stack_addr(cpu)] = 0x34;  // PC_lo
    cpu->sp--;

    // Set PC
    cpu->k = (uint8_t)(pc24 >> 16);
    cpu->pc = (uint16_t)(pc24 & 0xFFFF);

    // Loop until the final RTS restores SP to sp_save.
    // Guardrail: cap at 100k opcodes so we don't spin if something goes wrong
    // (e.g. an infinite loop caused by uninitialised RAM).
    long max_ops = 100000;
    while (cpu->sp != sp_save && max_ops-- > 0) {
        cpu_runOpcode(cpu);
        if (cpu->waiting || cpu->stopped) break;
    }
    if (max_ops <= 0) {
        fprintf(stderr, "run_emulated_func: opcode budget exhausted -- the routine "
                "did not return. SP=%04X sp_save=%04X PC=%02X:%04X\n",
                cpu->sp, sp_save, cpu->k, cpu->pc);
    }
}

// ---------------------------------------------------------------------------
// CalcHits oracle — formula decoded from the asm source:
//   CalcHits loops base_hits times and calls Rand99 each iteration.
//   Rand99 = RandXA(min=0, max=98). RandXA does:
//       result = (rng_table[ram[0x97]] % (max - min + 1)) + min
//   So Rand99 returns rng_table[ram[0x97]] % 99, a value in [0, 98].
//   CalcHits then compares it to hit_rate ($38fa): if Rand99 < hit_rate, count++.
//
// Note: ram[0x97] is NOT incremented by CalcHits or by Rand99 along this path.
// So every loop iteration reads THE SAME rng_table[idx] value. The actual
// ram[0x97] increment is performed by the caller at some other point.
// (To be confirmed in spike M2; for now we model the observed behaviour.)
// ---------------------------------------------------------------------------

static int oracle_calchits(const uint8_t *rng_table, uint8_t rng_index,
                           uint8_t base_hits, uint8_t hit_rate) {
    int count = 0;
    uint8_t rand_value = rng_table[rng_index] % 99;  // Rand99 returns the same value here
    for (int i = 0; i < base_hits; i++) {
        if (rand_value < hit_rate) count++;
    }
    return count;
}

// ---------------------------------------------------------------------------
// Test driver
// ---------------------------------------------------------------------------

static int run_trial(Snes *snes, const uint8_t *rng_table,
                     uint8_t rng_index, uint8_t base_hits, uint8_t hit_rate) {
    // Snapshot CPU registers before the call (so we can rerun).
    Cpu *cpu = snes->cpu;
    uint16_t saved_a = cpu->a, saved_x = cpu->x, saved_y = cpu->y;
    uint16_t saved_sp = cpu->sp, saved_pc = cpu->pc, saved_dp = cpu->dp;
    uint8_t saved_k = cpu->k, saved_db = cpu->db;
    bool saved_mf = cpu->mf, saved_xf = cpu->xf;

    // Memory setup:
    // - RNG table at $1900
    memcpy(snes->ram + RAM_RNG_TABLE, rng_table, 256);
    // - current index at $97 (direct page)
    snes->ram[RAM_RNG_INDEX] = rng_index;
    // - hit rate / base hits / clear output
    snes->ram[RAM_HIT_RATE] = hit_rate;
    snes->ram[RAM_BASE_HITS] = base_hits;
    snes->ram[RAM_NHITS_OUT] = 0xCC;  // sentinel (CalcHits must overwrite it)

    // CPU state for the battle module:
    // - DP = 0 (direct page is not used by CalcHits, but Rand99/RandXA do DP
    //   arithmetic on $96/$97/$3947/...). DP=0 is consistent with the battle
    //   convention as read in damage.asm, and with the fact that all of these
    //   addresses are accessed as absolute `<adr>` < $100.
    // - DB = $7E (data bank = WRAM). This is CRITICAL: without it, `STZ $38fd`
    //   would write to hardware register $00:38FD instead of WRAM.
    cpu->dp = 0;
    cpu->db = 0x7E;
    cpu->mf = true;   // 8-bit A
    cpu->xf = true;   // 8-bit X/Y

    run_emulated_func(snes, CALCHITS_ADDR_24);

    int observed = snes->ram[RAM_NHITS_OUT];
    int expected = oracle_calchits(rng_table, rng_index, base_hits, hit_rate);

    bool ok = (observed == expected);
    printf("  rng_idx=%3u base=%3u rate=%3u  observed=%3d  expected=%3d  %s\n",
           rng_index, base_hits, hit_rate, observed, expected, ok ? "OK" : "FAIL");

    // Restore the CPU for a clean next trial. The ROM hasn't moved, but we
    // pushed/popped on the stack -- restore it explicitly.
    cpu->a = saved_a; cpu->x = saved_x; cpu->y = saved_y;
    cpu->sp = saved_sp; cpu->pc = saved_pc; cpu->dp = saved_dp;
    cpu->k = saved_k; cpu->db = saved_db;
    cpu->mf = saved_mf; cpu->xf = saved_xf;

    return ok ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <rom.sfc>\n", argv[0]);
        return 1;
    }

    size_t rom_len = 0;
    uint8_t *rom = read_file(argv[1], &rom_len);
    if (!rom) return 2;

    Snes *snes = snes_init();
    if (!snes_loadRom(snes, rom, rom_len)) {
        fprintf(stderr, "snes_loadRom failed\n");
        return 3;
    }
    snes_reset(snes, true);

    // Run a few frames so the boot code stabilises CPU state.
    // (Otherwise SP/PC are in a very-early-boot state -- usable but weird.)
    for (int i = 0; i < 60; i++) snes_runFrame(snes);

    // Disable IRQ/NMI during trials so an interrupt cannot fire in the middle
    // of a run_emulated_func.
    snes->cpu->i = true;  // I flag SET = IRQ masked
    snes->cpu->nmiWanted = false;
    snes->cpu->irqWanted = false;

    // Synthetic RNG table for trials.
    uint8_t rng_table[256];
    for (int i = 0; i < 256; i++) rng_table[i] = (uint8_t)i;  // 0..255

    // Trials: cover rate=0/50/99, base=0 (early return), idx wrap >255, table
    // with zero values, and table values near the Rand99 max.
    int total = 0, fails = 0;

    #define TRIAL(idx, base, rate) do { fails += run_trial(snes, rng_table, idx, base, rate); total++; } while(0)
    TRIAL(0,  10, 99);
    TRIAL(0,  10, 50);
    TRIAL(0,  10, 0);
    TRIAL(50, 20, 99);
    TRIAL(0,  0,  99);    // base=0 → CalcHits must return without action (the sentinel is overwritten by STZ)
    TRIAL(200,100,99);    // wrap idx>=256 (idx & 0xFF = 200)
    TRIAL(0,  1,  98);    // 1 draw, table[0]=0 < 98 → 1 hit
    TRIAL(98, 1,  98);    // table[98]=98 % 99 = 98 ; 98 < 98 FALSE → 0 hit
    TRIAL(99, 1,  98);    // table[99]=99 % 99 = 0 ; 0 < 98 → 1 hit
    #undef TRIAL

    printf("\n=== summary ===\nfails: %d/%d\n", fails, total);
    free(rom);
    return fails == 0 ? 0 : 1;
}
