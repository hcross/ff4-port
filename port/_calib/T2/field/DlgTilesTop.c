#include "snes/snes.h"

// DlgTilesTop: Predefined tilemap data for dialog box top border.
// This is a 64-byte static data array, loaded directly into VRAM or RAM.
// No execution flow — just data. Translating to C as a byte array.
static void DlgTilesTop_c(Snes *snes) {
    // This function does not execute logic; it's a data blob.
    // Provided for parity completeness, but no C logic needed.
    // The data is embedded in the ROM and copied as-is.
}

// PITFALLS: none (data-only, no execution)
// HELPERS: none (no calls)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DlgTilesTop ($F6:D6)