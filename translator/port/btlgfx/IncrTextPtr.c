#include "snes/snes.h"
#include "snes/dma.h"

/* Standalone spike extraction of IncrTextPtr_c (= asm IncrTextPtr @ $02:A491),
 * bundled in ff4-gnw/battle/btlgfx_prim.c. Body copied verbatim.
 *
 * The routine is a 16-bit increment of the DP word at $30 (xf=false):
 *   A6 30   LDX $30    ; X = ram[$30:$31] (16-bit)
 *   E8      INX        ; 16-bit increment (wraps at 65536)
 *   86 30   STX $30    ; store both bytes
 *   60      RTS
 * X is 16-bit end-to-end, so there is no 8-bit register-width truncation to
 * reproduce (unlike BuildOAMEntries' 8-bit ASL index) — a plain uint16_t
 * increment matches the hardware exactly.
 *
 * snes_runCycles / dma_handleDma are the real LakeSnes functions (kept
 * verbatim, harmless): the spike compares post-state WRAM ($30=2), not cycle
 * timing, and HDMA targets PPU MMIO, never WRAM $30. NMI is disabled in the
 * spike harness (i=true, nmiWanted=false), so no re-entry can clobber $30. */

void IncrTextPtr_c(Snes *snes) {
    /* LDX_dp(32)+INX(14)+STX_dp(32)+RTS(42)=120, -16 pull = 104 */
    snes_runCycles(snes, 104);
    /* Fire HDMA (matching interpreter behaviour). */
    if (snes->dma->hdmaRunRequested || snes->dma->hdmaInitRequested)
        dma_handleDma(snes->dma, 8);
    uint16_t x = (uint16_t)(snes->ram[0x30] | ((uint16_t)snes->ram[0x31] << 8));
    x++;
    snes->ram[0x30] = (uint8_t)(x & 0xFF);
    snes->ram[0x31] = (uint8_t)(x >> 8);
    snes->cpu->x = x;
    snes->cpu->n = (x & 0x8000) != 0;
    snes->cpu->z = (x == 0);
}

// CONTRACT:
//   inputs_ram:  0x30=2
//   output_ram:  0x30=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: btlgfx::IncrTextPtr ($02:A491)
