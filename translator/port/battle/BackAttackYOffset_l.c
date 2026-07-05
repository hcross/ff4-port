#include "snes/snes.h"
#include "snes/cart.h"

/* Standalone spike extraction of BackAttackYOffset_l_c (= asm $02:BB1A),
 * bundled in ff4-gnw/battle/btlgfx_prim.c alongside BackAttackYOffset_s_c,
 * Mult8_btlgfx_c, HardMult_btlgfx_c and IncrTextPtr_c. Body copied verbatim
 * from the bundled file; only this routine's own code is reproduced here.
 *
 * asm $02:BB1A (8 opcodes, M=8) — identical structure to _s ($02:BB0B) but
 * subtracts $10 instead of $08:
 *   48        PHA
 *   AD C0 6C  LDA $6CC0          ; back-attack flag (absolute)
 *   F0 07     BEQ +7             ; no back attack -> PLA ; RTS
 *   68        PLA
 *   49 FF     EOR #$FF           ; A = ~A
 *   38        SEC
 *   E9 10     SBC #$10           ; A = ~A - $10
 *   60        RTS
 *   68        PLA                ; BEQ target
 *   60        RTS
 *
 * Sole output is register A + N/Z/C/V flags (the routine writes no WRAM; the
 * PHA/PLA pair leaves the stack net-unchanged). inject_cycles stubbed no-op;
 * snes_runCycles kept (real LakeSnes fn, harmless — no WRAM-writing HDMA/NMI
 * is driven mid-routine so cycle count is irrelevant to compared state).
 *
 * NOTE on the width class the skill flags: every arithmetic step here runs in
 * 8-bit accumulator mode (mf=true). EOR #$FF and SBC #$10 are 8-bit; the C
 * masks each intermediate back to uint8_t, so there is no ASL/shift-derived
 * index that could silently exceed 255. No truncation hazard in this routine. */

static inline void inject_cycles(Snes *snes, int n) { (void)snes; (void)n; }

void BackAttackYOffset_l_c(Snes *snes) {
    uint8_t a = (uint8_t)(snes->cpu->a);
    if (snes->ram[0x6CC0]) {
        /* identical structure to BB0B: 170 MC */
        snes_runCycles(snes, 170);
        uint8_t na = (uint8_t)(~a);
        uint8_t res = (uint8_t)(na - 0x10);
        snes->cpu->a  = (snes->cpu->a & 0xFF00) | res;
        snes->cpu->c  = (na >= 0x10);
        snes->cpu->v  = (((na ^ 0x10) & (na ^ res)) & 0x80) != 0;
        snes->cpu->n  = (res & 0x80) != 0;
        snes->cpu->z  = (res == 0);
    } else {
        /* identical structure to BB0B: 130 MC */
        snes_runCycles(snes, 130);
        snes->cpu->n = (a & 0x80) != 0;
        snes->cpu->z = (a == 0);
    }
    inject_cycles(snes, 0);
}

// SPIKE_OUTPUT_REG: a, c, z, n, v
// CONTRACT:
//   inputs_reg:  a=8
//   inputs_ram:  0x6CC0=1
//   output_ram:  none
//   mmio_effects: none
//   dma:         none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::BackAttackYOffset_l ($02:BB1A)
//
// The old `output_reg: a=8` line was a phantom field the generator silently
// ignored (never a real key in the Contract parser). SPIKE_OUTPUT_REG (added
// 2026-07-05) is the real mechanism -- see BackAttackYOffset_s.c's identical
// note and translator/runs/D02BB0B_backattackyoffset_s_BLOCKED_vacuous_spike.txt.
