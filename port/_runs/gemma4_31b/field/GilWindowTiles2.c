#include "snes/snes.h"

// This routine is not a functional block of code, but a data table
// containing tile indices for the Gil window display.
// It is accessed as data by other routines rather than executed.
// Since it is a static data table in the ROM, the C reimplementation
// returns the value at the requested offset from the table base.
static uint8_t GilWindowTiles2_get(Snes *snes, int offset) {
    // ROM data at $F6:0E
    // The table differs based on language (EN vs JP). 
    // The provided ASM shows the EN branch and the else branch.
    // Based on the port targets, we implement the EN version.
    static const uint8_t table[] = {
        0x19, 0x20, 0xFF, 0x20, 0xFF, 0x20, 0xFF, 0x20,
        0xFF, 0x20, 0xFF, 0x20, 0xFF, 0x20, 0xFF, 0x20,
        0xFF, 0x20, 0xFF, 0x20, 0xFF, 0x20, 0x1A, 0x20
    };

    if (offset < 0 || offset >= (int)(sizeof(table) / sizeof(table[0]))) {
        return 0;
    }
    return table[offset];
}

// PITFALLS: None. This is a data table, not an executable routine.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (This is a data table; parity is verified by ROM read checks)

// REVERSED_FUNCTION: field::GilWindowTiles2 ($F6:0E)