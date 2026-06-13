#include "snes/snes.h"

// MapGfxBankTbl is a compile-time generated table of 16 bytes,
// each byte corresponds to the bank number of MapGfx_0000 through MapGfx_000F.
// The table is embedded directly in the ROM at $B1:04.
// This is a data table, not a routine, so no execution logic is needed.
// The C equivalent is a const array.

// PITFALLS: none (this is a data table, not a routine)
// HELPERS: none (this is a data table, not a routine)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
static void MapGfxBankTbl_c(Snes *snes) {
    // This function does nothing because MapGfxBankTbl is a data table.
    // The table is defined in ROM as:
    // .repeat 16, i
    //   .bankbytes .ident(.sprintf("MapGfx_%04x", i))
    // .endrep
    // Which expands to the bank bytes of MapGfx_0000 to MapGfx_000F.
    // In C, this would be represented as a const array, but since this is
    // a parity translation task and the original is just data, we do nothing.
}

// REVERSED_FUNCTION: field::MapGfxBankTbl ($B1:0004)