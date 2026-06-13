#include "snes/snes.h"

// This routine is a simple jump wrapper that redirects execution
// to the main UpdateScrollRegs logic.
static void UpdateScrollRegs_ext_c(Snes *snes) {
    update_scroll_regs_emu(snes);
}

// PITFALLS: None.
// HELPERS: update_scroll_regs_emu(snes) — delegates UpdateScrollRegs @ $FF:0A
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::UpdateScrollRegs_ext ($FD:0C)