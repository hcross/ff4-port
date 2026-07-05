#include "snes/snes.h"

/* Standalone spike extraction of BackAttackYOffset_s_c (= asm BackAttackFlipX
 * @ $02:BB0B), bundled in ff4-gnw/battle/btlgfx_prim.c alongside four other
 * routines. Body copied verbatim from the dispatched source (and identical to
 * the sub-call already inlined in BuildOAMEntries.c). snes_runCycles kept (real
 * LakeSnes fn, harmless): the routine has NO WRAM footprint, its only output is
 * register A + the NZVC flags.
 *
 * asm (10 opcodes, M=8):
 *   48        PHA
 *   AD C0 6C  LDA $6CC0       ; back-attack flag (absolute, DB:$6CC0)
 *   F0 07     BEQ +7          ; no back attack -> PLA ; RTS
 *   68        PLA
 *   49 FF     EOR #$FF        ; ~A
 *   38        SEC
 *   E9 08     SBC #$08        ; A = ~A - 8
 *   60        RTS
 *   68        PLA             ; BEQ target
 *   60        RTS
 */
void BackAttackYOffset_s_c(Snes *snes) {
    uint8_t a = (uint8_t)(snes->cpu->a);
    if (snes->ram[0x6CC0]) {
        /* PHA(22)+LDA(32)+BEQ_nt(16)+PLA(28)+EOR(16)+SEC(14)+SBC(16)+RTS(42)=186, -16 pull = 170 */
        snes_runCycles(snes, 170);
        uint8_t na = (uint8_t)(~a);
        uint8_t res = (uint8_t)(na - 8);
        snes->cpu->a  = (snes->cpu->a & 0xFF00) | res;
        snes->cpu->c  = (na >= 8);
        snes->cpu->v  = (((na ^ 8) & (na ^ res)) & 0x80) != 0;
        snes->cpu->n  = (res & 0x80) != 0;
        snes->cpu->z  = (res == 0);
    } else {
        /* PHA(22)+LDA(32)+BEQ_t(22)+PLA(28)+RTS(42)=146, -16 pull = 130 */
        snes_runCycles(snes, 130);
        snes->cpu->n = (a & 0x80) != 0;
        snes->cpu->z = (a == 0);
    }
}

// CONTRACT:
//   inputs_reg:  a=8
//   inputs_ram:  0x6CC0=1
//   output_ram:  none
//   mmio_effects: none
//   dma:         none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: btlgfx::BackAttackFlipX ($02:BB0B)
