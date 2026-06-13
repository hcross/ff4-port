#include "snes/snes.h"

// Entry mode: Inherited from caller (typically mf=true, xf=false, dp=0, db=$7E)
// This routine is a simple jump wrapper to the main controller initialization.
static void InitCtrl_ext2_c(Snes *snes) {
    init_ctrl_emu(snes); // jmp InitCtrl
}

// PITFALLS: None.
// HELPERS: init_ctrl_emu(snes) — delegates InitCtrl @ $FD:D9
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::InitCtrl_ext2 ($FD:00)