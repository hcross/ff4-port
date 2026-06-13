#include "snes/snes.h"

// This routine is a simple jump wrapper that redirects execution to SaveDlgGfx.
static void SaveDlgGfx_ext_c(Snes *snes) {
    save_dlg_gfx_emu(snes); // jmp SaveDlgGfx
}

// PITFALLS: None.
// HELPERS: save_dlg_gfx_emu(snes) — delegates SaveDlgGfx @ $FF:62
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFD
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: menu::SaveDlgGfx_ext ($FD:0F)