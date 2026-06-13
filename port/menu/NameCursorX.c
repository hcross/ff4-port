#include "snes/snes.h"

// This routine is a lookup table (data array) rather than executable code.
// It provides the X-coordinates for name cursors in the menu.
// In a C reimplementation, this is represented as a static array 
// or a function that returns a value from the table based on an index.
// Based on the ASM provided, the routine at $FF:CC is actually a table of bytes.
static uint8_t get_name_cursor_x(Snes *snes, uint8_t index) {
    static const uint8_t table[] = {
        0x38, 0x48, 0x58, 0x68, 0x78, 0x90, 0xA0, 0xB0, 0xC0, 0xD0
    };

    if (index >= 10) return 0; // Safety bounds check
    return table[index];
}

// PITFALLS: None. This is a data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none (assumed index passed as arg)
//   inputs_ram:  none
//   output_ram:  none (returns value via A register in original asm context)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFF
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: menu::NameCursorX ($FF:CC)