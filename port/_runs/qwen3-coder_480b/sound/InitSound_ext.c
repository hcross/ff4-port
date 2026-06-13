#include "snes/snes.h"

static void InitSound_ext_c(Snes *snes) {
    // This routine is a thin wrapper that calls InitSound and then returns
    // via RTL (return from subroutine long), meaning it was called with JSL.
    // No specific register or flag setup is required as it just forwards
    // control to InitSound and returns.

    init_sound_emu(snes);  // jsr InitSound
    // RTL is handled by the caller framework in the emulation layer
}

// PITFALLS: none (simple delegation, no memory ops or flag dependencies)
// HELPERS: init_sound_emu(snes) — delegates InitSound @ $8088
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=auto, xf=auto, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: sound::InitSound_ext ($80:0000)