#include "snes/snes.h"

// The routine ProphecyPal is actually a data table of 16-bit words
// rather than executable code. Based on the ASM source, it is a 
// sequence of values used for palette mapping or color offsets.
// Since the task asks for a translation of this "routine" and it 
// contains only .word directives, we implement it as a 
// read-only table accessor to maintain semantic parity.

static uint16_t get_prophecy_pal_value(Snes *snes, uint16_t index) {
    // The table is located at $F9:96. 
    // Indices are typically provided by the caller (e.g., via X or Y).
    // Each element is 2 bytes.
    const uint16_t table[] = {
        0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000,
        0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000,
        0x4000, 0x4400, 0x4800, 0x4C00, 0x5000, 0x5400, 0x5800, 0x5C00,
        0x6000, 0x6400, 0x6800, 0x6C00, 0x7000, 0x7400, 0x7800, 0x7C00
    };

    if (index >= 32) return 0; // Bounds check for safety
    return table[index];
}

// PITFALLS: None. This is a data table, not a logic routine.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (This is a data table; parity is checked by reading the values)

// REVERSED_FUNCTION: field::ProphecyPal ($F9:96)