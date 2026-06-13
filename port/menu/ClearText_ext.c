#include "snes/snes.h"

// This routine is a simple jump wrapper that redirects execution 
// to the main ClearText routine.
static void ClearText_ext_c(Snes *snes) {
    clear_text_emu(snes); // jmp ClearText
}

// PITFALLS: None.
// HELPERS: clear_text_emu(snes) — delegates ClearText @ $FE:9E
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFD
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: menu::ClearText_ext ($FD:06)