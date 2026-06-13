#include "snes/snes.h"

// This "routine" is actually a data table (lookup table) containing 32 
// bytes of rotation values. In the C reimplementation, this is 
// translated as a static constant array to be accessed by the 
// routines that index into this memory region.
static const uint8_t TunnelRotTbl[] = {
    0x01, 0x02, 0x04, 0x06, 0x09, 0x0C, 0x10, 0x14, 
    0x19, 0x1E, 0x23, 0x29, 0x30, 0x36, 0x3E, 0x45,
    0x4E, 0x56, 0x5F, 0x69, 0x73, 0x7D, 0x88, 0x94, 
    0x9F, 0xAC, 0xB8, 0xC6, 0xD3, 0xE1, 0xF0, 0xFF
};

// PITFALLS: None. This is a data definition, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  n/a
//   entry_flags: n/a
//   CUSTOM_SPIKE: yes (Data table, not a function)

// REVERSED_FUNCTION: field::TunnelRotTbl ($FA:B6)