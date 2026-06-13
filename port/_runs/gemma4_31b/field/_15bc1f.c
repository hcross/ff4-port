#include "snes/snes.h"

// This routine is not a function, but a data table (lookup table).
// The ASM provided consists only of .byte directives.
// Based on the address $BC:1F, this is a data block.
// In a C reimplementation, this is translated as a static const array.
static const uint8_t _15bc1f_table[] = {
    0x00, 0x00, 0x44, 0x3E,
    0x08, 0x00, 0x45, 0x3E,
    0x00, 0x08, 0x46, 0x3E,
    0x08, 0x08, 0x47, 0x3E
};

// Since this is a data table and not an executable routine, 
// it does not have a C function body, register inputs, or flags.
// It is accessed by other routines via absolute addressing.

// PITFALLS: None (Data block)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram: none
//   entry_mode:  none
//   entry_flags: none
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::_15bc1f ($BC:1F)