#include "snes/snes.h"

// This routine is a simple jump wrapper that redirects execution to UpdateCtrl.
// It preserves all registers and flags.
static void UpdateCtrl_ext_c(Snes *snes) {
    update_ctrl_emu(snes);
}

// PITFALLS: None.
// HELPERS: update_ctrl_emu(snes) — delegates UpdateCtrl @ $FD:D0
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0, db=0xFD
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::UpdateCtrl_ext ($FD:03)