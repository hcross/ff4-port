#include "snes/snes.h"

// This routine is a DEBUG-only wrapper. In the release build,
// it simply returns. In DEBUG builds, it calls DrawPos.
// Since the target is the final native reimplementation, we 
// maintain the logical structure.
static void _15c23d_c(Snes *snes) {
    // The original asm contains a conditional .if DEBUG block.
    // If the build target is non-debug, this function is effectively a NOP.
#ifdef DEBUG
    draw_pos_emu(snes);
#endif
}

// PITFALLS: None applicable (trivial routine).
// HELPERS: draw_pos_emu(snes) — delegates DrawPos @ $C2:276
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: auto

// REVERSED_FUNCTION: field::_15c23d ($C2:3D)