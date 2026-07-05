#include "snes/snes.h"

/* Standalone spike extraction of Mult8_btlgfx_c (= asm 8-bit multiply
 * @ $02:8560), bundled in ff4-gnw/battle/btlgfx_prim.c. Body copied verbatim.
 *
 * $02:8560 is an 8x8 shift-add multiply (product in $2A:$2B):
 *   DA        PHX
 *   A2 08 00  LDX #$0008     ; 8 iterations (16-bit X, xf=0)
 *   64 2A     STZ $2A        ; product lo
 *   64 2B     STZ $2B        ; partial-sum / product hi
 *   66 28     ROR $28        ; shift multiplier, bit0 -> carry, carry-in -> bit7
 *   90 07     BCC +7         ; bit clear -> skip add
 *   A5 26     LDA $26        ; multiplicand
 *   18        CLC
 *   65 2B     ADC $2B
 *   85 2B     STA $2B
 *   66 2B     ROR $2B        ; shift partial-sum, carry -> product
 *   66 2A     ROR $2A        ; shift product; its carry-out feeds next ROR $28
 *   CA        DEX
 *   D0 EE     BNE loop
 *   FA        PLX
 *   60        RTS
 *
 * Width note (the register-width truncation bug class): this routine is a
 * bit-serial ROR/ADC loop, NOT an 8-bit-mode shift-derived index
 * (`TXA;ASL;ASL;TAY`) like BuildOAMEntries. There is no `slot << 2` to
 * truncate at 8 bits — every operation is an explicit single-bit rotate or an
 * 8-bit ADC, and the C mirrors each one at exactly the width the asm computes
 * in (uint8_t rotates, a uint16_t sum only to extract the ADC carry-out). So
 * the (uint16_t)(uint8_t) truncation discipline that BuildOAMEntries needed
 * does not apply here; the danger instead is the carry chain (below).
 *
 * Carry chain: the carry flowing into each `ROR $28` is the carry-OUT of the
 * previous `ROR $2A`, not $28's own bits. The first `ROR $28` also folds the
 * entry carry flag (cpu->c at call time) into bit7 of $28 — so cpu->c is a
 * genuine input. The spike does not fuzz cpu->c (the CONTRACT schema has no
 * carry-in field), but both the asm and C passes see the same baseline carry,
 * so the comparison stays consistent. The only faithful model is to simulate
 * the whole loop so the carry chain is exact (done below).
 *
 * snes_runCycles is the real LakeSnes function (kept verbatim, harmless): the
 * spike compares post-state WRAM ($2A:$2B), not cycle timing, and the routine
 * writes only zero-page ($2A/$2B/$28) which no HDMA/DMA can touch. NMI is
 * disabled in the spike harness (i=true, nmiWanted=false), so the on-device
 * re-entry hazard the save-inputs-first logic guards against cannot fire here;
 * the saved values equal the live ones, so the result is identical. */

void Mult8_btlgfx_c(Snes *snes) {
    /* Save inputs BEFORE snes_runCycles — a VBlank NMI firing inside that call
     * can re-enter Mult8_btlgfx_c (via PeriodicMenuUpdate->LoadMenuTfrData->Mult8)
     * and clobber $26/$28 in WRAM and cpu->c. */
    uint8_t multiplicand = snes->ram[0x26];
    uint8_t mult         = snes->ram[0x28];
    uint8_t carry_in     = snes->cpu->c;  /* save before NMI can clobber it */

    /* Cycle-accurate accounting: mirrors the 8-iteration ROR-based loop.
     * PHX(30)+LDX_imm(24)+STZ_2A(24)+STZ_2B(24) = 102 preamble
     * Per iter (bit=0, BCC taken): ROR_28(38)+BCC_t(22)+ROR_2B(38)+ROR_2A(38)+DEX(14)+BNE(22/16)
     * Per iter (bit=1, BCC not taken): ROR_28(38)+BCC_nt(16)+LDA(24)+CLC(14)+ADC(24)+STA(24)+ROR_2B(38)+ROR_2A(38)+DEX(14)+BNE(22/16)
     * PLX(36)+RTS(42) = 78 epilog.  Subtract 16 MC already consumed by dispatch pullWord. */
    int cyc = 102 + 78;
    {
        uint8_t tmp = mult;
        for (int i = 0; i < 8; i++) {
            cyc += 38; /* ROR $28 */
            if (tmp & 1) {
                cyc += 16 + 24 + 14 + 24 + 24; /* BCC_nt + LDA + CLC + ADC + STA */
            } else {
                cyc += 22; /* BCC taken */
            }
            cyc += 38 + 38 + 14; /* ROR $2B + ROR $2A + DEX */
            cyc += (i < 7) ? 22 : 16; /* BNE taken / not taken on last iter */
            tmp >>= 1;
        }
    }
    snes_runCycles(snes, cyc - 16);
    /* After this point $26/$28/cpu->c may be clobbered by an NMI re-entry. */

    /* Full shift-and-add simulation matching $02:8560 exactly:
     *   STZ $2A; STZ $2B; LDX #8
     *   loop: ROR $28; BCC nc; CLC; ADC $26->$2B; nc: ROR $2B; ROR $2A; DEX; BNE
     *
     * The carry flowing into each ROR $28 comes from the PREVIOUS ROR $2A,
     * NOT from $28's own bits. Simulating ROR $28 in isolation (as a plain
     * 8xROR loop) produces the wrong $28 side-effect and a wrong carry flag.
     * The only correct approach is to simulate the full loop so the carry
     * chain is faithful. */
    uint8_t c     = carry_in;
    uint8_t m     = mult;
    uint8_t p_lo  = 0;   /* $2A — STZ $2A */
    uint8_t p_hi  = 0;   /* $2B — STZ $2B */

    for (int i = 0; i < 8; i++) {
        /* ROR $28: bit0 of m -> carry; incoming carry -> bit7 of m */
        uint8_t bit28 = m & 1;
        m  = (m >> 1) | (c << 7);
        c  = bit28;  /* carry = old bit0($28) */

        /* BCC: if carry (old bit0($28)) add multiplicand to p_hi.
         * CLC before ADC means carry-out = plain overflow bit. */
        if (c) {
            uint16_t sum = (uint16_t)p_hi + (uint16_t)multiplicand;
            p_hi = (uint8_t)(sum & 0xFF);
            c    = (uint8_t)(sum >> 8);
        }
        /* else BCC taken: c already 0, no add */

        /* ROR $2B */
        uint8_t bit_hi = p_hi & 1;
        p_hi = (p_hi >> 1) | (c << 7);
        c    = bit_hi;

        /* ROR $2A — carry from ROR $2B feeds next iter's ROR $28 */
        uint8_t bit_lo = p_lo & 1;
        p_lo = (p_lo >> 1) | (c << 7);
        c    = bit_lo;
    }

    snes->ram[0x28] = m;
    snes->ram[0x2A] = p_lo;
    snes->ram[0x2B] = p_hi;
    snes->cpu->c = c;
    /* PLX sets N/Z from the restored (original) X value. */
    snes->cpu->n = (snes->cpu->x & 0x8000) != 0;
    snes->cpu->z = (snes->cpu->x == 0);
}

// CONTRACT:
//   inputs_ram:  0x26=1, 0x28=1
//   output_ram:  0x2A=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: btlgfx::Mult8_btlgfx ($02:8560)
