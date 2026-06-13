#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x0, DP=0
// Purpose: Simple wrapper that delegates the wait for a Vblank event to WaitVblankLong.
static void WaitVblankEvent_c(Snes *snes) {
    wait_vblank_long_emu(snes); // jsr WaitVblankLong
}

// PITFALLS: None relevant for this wrapper.
// HELPERS: wait_vblank_long_emu(snes) — delegates WaitVblankLong @$00:912D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::WaitVblankEvent ($E3:5B)