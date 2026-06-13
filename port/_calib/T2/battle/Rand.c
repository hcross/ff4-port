#include "snes/snes.h"

// Rand: wrapper for RandXA that generates a random value in 0..255
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: none (no inputs in registers)
// Output: A = random byte (0-255)
static void Rand_c(Snes *snes) {
    // clr_ax → tdc / tax (transfers D=0 to A and X)
    // lda #$ff → A = 0xFF
    // jsr RandXA → calls RandXA with X=0, A=0xFF
    randxa_emu(snes);  // returns result in A
}

// PITFALLS: 5 (clr_ax is tdc/tax, not lda #0), 1 (DB=$7E for battle)
// HELPERS: randxa_emu(snes) — delegates RandXA @ $03:8379
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::Rand ($03:8593)