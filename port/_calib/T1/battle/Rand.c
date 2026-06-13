#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$85, DP=0
// This routine serves as a wrapper for RandXA, requesting a random value 
// with a range defined by $FF.
// Logic:
//   1. Clear A and X (A=0, X=0) via clr_ax (TDC/TAX)
//   2. Load A with 0xFF
//   3. Call RandXA to generate the random value
static uint16_t Rand_c(Snes *snes) {
    // clr_ax: A = 0, X = 0 (Assuming DP=0)
    snes->cpu->a = 0;
    snes->cpu->x = 0;

    // lda #$ff
    snes->cpu->a = 0xFF;

    // jsr RandXA (delegated)
    // The return value of the emulator helper is the final A register
    return randxa_emu(snes);
}

// PITFALLS: 5 (clr_ax is tdc/tax, effectively zeroing A and X when DP=0)
// HELPERS: randxa_emu(snes) — delegates RandXA @ $85:8379
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram: none
//   output_ram: none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x85
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (Returns value via register A)

// REVERSED_FUNCTION: battle::Rand ($85:93)