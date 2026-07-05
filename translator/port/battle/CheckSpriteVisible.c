#include "snes/snes.h"
#include "snes/cart.h"

/* Standalone spike extraction of CheckSpriteVisible_c (asm @ $02:DDA5),
 * bundled in ff4-gnw/battle/btlgfx_monsters.c. Body copied verbatim.
 * inject_cycles is stubbed no-op: the region spike compares post-state, not
 * cycle timing (this routine fires no WRAM-writing HDMA/NMI).
 *
 * HARNESS SCOPE NOTE — the routine's *primary* output is the carry flag
 * (C=0 => sprite visible, C=1 => hidden). generate_spike.py compares WRAM
 * only; it cannot observe the carry flag. This spike therefore proves the
 * WRAM-observable contract: ram[$64] parity (INC on the dead/absent path)
 * plus "the C body writes no WRAM the asm doesn't" (region compare, $0E
 * scratch masked). Carry-flag equivalence rests on the prior whole-game
 * oracle validation (MemPalace ff4-gnw, handoff_key=ff4-port-frame26-debug,
 * 2026-06-24: "CheckSpriteVisible_c ($02:DDA5) — 4 paths : 232/258/598/592
 * MC", 0 WRAM diff on savestates 002/005/006). */

static inline void inject_cycles(Snes *snes, int n) { (void)snes; (void)n; }

void CheckSpriteVisible_c(Snes *snes) {
    uint16_t saved_x = snes->cpu->x;  /* PHX saves original X; PLX restores it on all paths */
    uint8_t mon = snes->ram[0x47];

    if (snes->ram[0xF0AF + mon]) {
        /* Path A: dead/absent => INC $64, C=1, PLX restores X
         * PHX(30)+LDA_dp(24)+TAX(14)+LDA_abs_X(38)+BNE_t(22)+INC_dp(38)+PLX(36)+SEC(14)+RTS(42)
         * = 258 MC; -16 JSR sim = 242 */
        inject_cycles(snes, 242);
        snes->ram[0x64]++;
        snes->cpu->c = 1;
        snes->cpu->x = saved_x;
        snes->cpu->n = (saved_x & 0x8000) != 0;
        snes->cpu->z = (saved_x == 0);
        return;
    }

    if (snes->ram[0xF46D + mon]) {
        /* Path B: other hidden flag => C=1
         * PHX(30)+LDA_dp(24)+TAX(14)+LDA_abs_X(38)+BNE_nt(16)+LDA_abs_X(38)+BNE_t(22)
         * +PLX(36)+SEC(14)+RTS(42) = 274 MC; -16 JSR sim = 258 */
        inject_cycles(snes, 258);
        snes->cpu->c = 1;
        snes->cpu->x = saved_x;
        snes->cpu->n = (saved_x & 0x8000) != 0;
        snes->cpu->z = (saved_x == 0);
        return;
    }

    /* monster*4. asm computes the index in an 8-bit accumulator
     * (LDA $47/ASL/ASL/TAX, mf=true) so it wraps at 256; truncate to 8-bit
     * BEFORE widening to match (benign for the real 0..5 range, but closes
     * the latent width-truncation gap for mon>=64 — same bug class as
     * BuildOAMEntries_c's y_row). */
    uint16_t xi = (uint16_t)(uint8_t)(mon << 2);
    uint8_t attr = (snes->ram[0xF015 + xi] & 0xF7)
                 | (snes->ram[0xF016 + xi] & 0xB8)
                 | (snes->ram[0xF018 + xi] & 0x01);

    /* PLX restores X; N/Z set from PLX (reflects saved_x) */
    /* A is updated by the ORA chain; we restore: A = attr byte */
    snes->cpu->a = (snes->cpu->a & 0xFF00) | attr;
    snes->cpu->x = saved_x;
    snes->cpu->n = (saved_x & 0x8000) != 0;
    snes->cpu->z = (saved_x == 0);

    if (attr) {
        /* Path C: visible => C=0
         * PHX(30)+LDA_dp(24)+TAX(14)+LDA_abs_X(38)+BNE_nt(16)+LDA_abs_X(38)+BNE_nt(16)
         * +LDA_dp(24)+ASL(14)+ASL(14)+TAX(14)+LDA_abs_X(38)+AND_imm(16)+STA_dp(24)
         * +LDA_abs_X(38)+AND_imm(16)+ORA_dp(24)+STA_dp(24)+LDA_abs_X(38)+AND_imm(16)
         * +ORA_dp(24)+BNE_t(22)+PLX(36)+CLC(14)+RTS(42) = 614 MC; -16 JSR sim = 598
         * NOTE: LDA abs,X = 38 MC with X=16-bit (cpu_adrAbx: idle always when xf=0) */
        inject_cycles(snes, 598);
        snes->cpu->c = 0;
    } else {
        /* Path D: not visible => C=1
         * Same as C but BNE_nt(16)+PLX(36)+SEC(14)+RTS(42) = 608 MC; -16 JSR sim = 592 */
        inject_cycles(snes, 592);
        snes->cpu->c = 1;
    }
}

// SPIKE_COMPARE: region
// SPIKE_MASK: 0x0E-0x0E
// CONTRACT:
//   inputs_ram:  0x47=1, 0xF0AF=1, 0xF46D=1, 0xF015=1, 0xF016=1, 0xF018=1
//   output_ram:  0x64=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: btlgfx::CheckSpriteVisible ($02:DDA5)
