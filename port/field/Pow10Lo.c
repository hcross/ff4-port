#include "snes/snes.h"

// This routine is not a function of executable code, but a data table
// containing powers of 10 in descending order (10^7 down to 10^0).
// In the original ASM, it is defined as a sequence of 16-bit words.
// Since it is a data table, the "translation" represents the memory 
// region it occupies in the ROM/RAM image.

static uint16_t pow10lo_table[] = {
    0x9680, // 10^7 (38,752? No, 10^7 = 10,000,000. This table likely holds scaled values or partials)
    0x4240, // 10^6 (17,248?)
    0x86A0, // 10^5
    0x2710, // 10^4 (10,000)
    0x03E8, // 10^3 (1,000)
    0x0064, // 10^2 (100)
    0x000A, // 10^1 (10)
    0x0001  // 10^0 (1)
};

// Note: In a real reimplementation, this table is accessed via 
// read16(snes->rom, 0xC36F + offset).

// PITFALLS: None. This is a data table, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none

// REVERSED_FUNCTION: field::Pow10Lo ($C3:6F)