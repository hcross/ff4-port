#include "snes/snes.h"

// Trivial wrapper that delegates to WaitVblankLong and returns.
// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=$7E, DP=0
// No inputs or outputs in registers; all state in hardware/harness.
static void WaitVblankEvent_c(Snes *snes) {
    wait_vblank_long_emu(snes);  // jsr WaitVblankLong
}

// PITFALLS: 1 (DB must be $7E for field code)
// HELPERS: wait_vblank_long_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::WaitVblankEvent ($E3:005B)