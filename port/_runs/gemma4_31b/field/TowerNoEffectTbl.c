#include "snes/snes.h"

// This "routine" is actually a data table (lookup table) in the ROM.
// In the context of a C reimplementation, this is translated as a 
// constant array of 16-bit words.
static const uint16_t TowerNoEffectTbl[] = {
    0x01f7, 0x0112, 0x008e, 0x000b, 0x0007, 0x0005, 0x0002, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

// PITFALLS: None. This is a static data table, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none
//   CUSTOM_SPIKE: yes (This is a data table; parity is verified by 
//                    comparing the array values against the ROM bytes).

// REVERSED_FUNCTION: field::TowerNoEffectTbl ($F8:06)