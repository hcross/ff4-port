#include "snes/snes.h"

// This routine is a wrapper that conditionally calls DrawPos based on a 
// DEBUG flag. In the production ROM, this is typically an empty routine 
// that simply returns (RTL).
static void _15c23d_c(Snes *snes) {
    // The .if DEBUG block is a compile-time conditional in the original ASM.
    // For the native reimplementation, this is effectively a no-op 
    // unless the project's global DEBUG state is active.
#if defined(DEBUG)
    draw_pos_emu(snes);
#endif
}

// PITFALLS: None (routine is essentially a no-op in release builds)
// HELPERS: draw_pos_emu(snes) — delegates DrawPos @ $C2:276
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::_15c23d ($C2:3D)