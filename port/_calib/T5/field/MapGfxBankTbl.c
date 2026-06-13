#include "snes/snes.h"

// MapGfxBankTbl is a ROM data table, not executable code.
// The original asm emits 16 24-bit pointers (bank + address) via .bankbytes.
// This C function is a no-op placeholder; the actual data is provided as a
// separate constant array (kMapGfxBankTbl) for other translated functions.
static void MapGfxBankTbl_c(Snes *snes) {
    (void)snes;
}

// PITFALLS: none (data table, no code)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
//   CUSTOM_SPIKE: yes (data table, not a function; human must write spike)
// REVERSED_FUNCTION: field::MapGfxBankTbl ($B1:04)