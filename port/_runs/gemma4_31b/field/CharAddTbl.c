#include "snes/snes.h"

// This "routine" is actually a data table (lookup table) rather than 
// executable code. It defines a mapping of characters or values.
// In the C reimplementation, this is represented as a constant array.
static const uint8_t CharAddTbl[] = {
    0x80, 0x00, 0x04, 0x01, 0x80, 0x02, 0x03, 0x80,
    0x80, 0x01, 0x80, 0x80, 0x80, 0x01, 0x00, 0x80,
    0x80, 0x80, 0x01, 0x80
};

// PITFALLS: None. This is a data block.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: auto
// CUSTOM_SPIKE: yes (This is a table, not a function; parity is verified by 
// checking the memory content at the specified ROM address).

// REVERSED_FUNCTION: field::CharAddTbl ($E9:A5)