#include "snes/snes.h"

// This routine is a thin wrapper that calls InitCharProp and returns.
// Logic:
//   call InitCharProp (delegated)
//   return
static void InitCharProp_ext_c(Snes *snes) {
    init_char_prop_emu(snes);
}

// PITFALLS: None. Simple wrapper.
// HELPERS: init_char_prop_emu(snes) — delegates InitCharProp @$94EE
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0, db=0xFF
//   entry_flags: auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::InitCharProp_ext ($FF:BC)