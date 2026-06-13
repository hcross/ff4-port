#include "snes/snes.h"

// This routine is a data table (byte array) rather than executable code.
// It defines a sequence of tile indices and spacing (0x20) for the Gil window.
// Since it consists only of .byte directives, it does not perform any logic.
// In the context of the C reimplementation, this is treated as a constant array.
static const uint8_t GilWindowTiles4_data[] = {
    0x1b, 0x20, 0x1c, 0x20, 0x1c, 0x20, 0x1c, 0x20,
    0x1c, 0x20, 0x1c, 0x20, 0x1c, 0x20, 0x1c, 0x20,
    0x1c, 0x20, 0x1c, 0x20, 0x1c, 0x20, 0x1d, 0x20
};

// To maintain parity with the ASM-based access pattern, 
// any C-side reader will access this data via the ROM address $F6:3E.
// If the harness requires a function wrapper for a "routine" that is actually data:
static void GilWindowTiles4_c(Snes *snes) {
    // No-op: this is a data table.
    (void)snes;
}

// PITFALLS: None. This is a static data block.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GilWindowTiles4 ($F6:3E)