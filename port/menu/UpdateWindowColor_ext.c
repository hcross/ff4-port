#include "snes/snes.h"

// This routine is a simple jump wrapper that redirects execution
// to the main UpdateWindowColor routine.
static void UpdateWindowColor_ext_c(Snes *snes) {
    update_window_color_emu(snes);
}

// PITFALLS: None (Direct jump)
// HELPERS: update_window_color_emu(snes) — delegates UpdateWindowColor @ $FD:E6
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFD
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::UpdateWindowColor_ext ($FD:09)