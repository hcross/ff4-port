#include "snes/snes.h"
#include "snes/cart.h"

/* Standalone spike extraction of HardMult_btlgfx_c (= asm MultHW @ $02:85D2),
 * bundled in ff4-gnw/battle/btlgfx_prim.c alongside four other primitives.
 * The body is copied verbatim; snes_runCycles(398) is replaced by a no-op
 * inject_cycles because this spike compares post-state ($20:$21), not cycle
 * timing — the SNES hardware 8x8 multiply is instantaneous in emulation, so
 * cycle count is irrelevant to the WRAM output the spike checks.
 *
 * asm ($02:85D2, M=8, X=16, DP=0):
 *   PHX / LDA $1C / STA $004202 (MPYA) / LDA $1E / STA $004203 (MPYB) /
 *   PHB / TDC / PHA / PLB / LDX $4216 (RDMPYL:RDMPYH, 16-bit product) /
 *   STX $20 / PLB / PLX / RTS
 * i.e. ram[$20]:ram[$21] = ram[$1C] * ram[$1E]  (unsigned 8x8 -> 16-bit).
 * The product is 8x8 -> 16, so it always fits uint16_t: no register-width
 * truncation hazard here (unlike the 8-bit ASL index class in BuildOAMEntries). */

static inline void inject_cycles(Snes *snes, int n) { (void)snes; (void)n; }

void HardMult_btlgfx_c(Snes *snes) {
    /* Save inputs before the (stubbed) cycle call — same NMI-clobber hazard as
     * Mult8_btlgfx_c in the live body; harmless but kept for fidelity. */
    uint8_t mc = snes->ram[0x1C];
    uint8_t mp = snes->ram[0x1E];
    inject_cycles(snes, 398);  /* timing irrelevant for the post-state spike */
    uint16_t product = (uint16_t)mc * (uint16_t)mp;
    snes->ram[0x20] = (uint8_t)(product & 0xFF);
    snes->ram[0x21] = (uint8_t)(product >> 8);
    snes->cpu->a = snes->cpu->dp;                 /* TDC: dp=0 -> A=0 */
    snes->cpu->n = (snes->cpu->x & 0x8000) != 0;  /* PLX sets N/Z from X */
    snes->cpu->z = (snes->cpu->x == 0);
}

// CONTRACT:
//   inputs_ram:  0x1C=1, 0x1E=1
//   output_ram:  0x20=2
//   mmio_effects: 4202, 4203
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: btlgfx::MultHW ($02:85D2)
