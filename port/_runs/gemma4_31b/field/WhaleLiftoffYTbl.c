#include "snes/snes.h"

// This "routine" is actually a data table (lookup table) used for Whale lift-off Y coordinates.
// In the C reimplementation, this is translated as a static constant array.
// Since it is a data table, it does not have a function body, but is accessed via 
// memory offsets in the original ASM.

static const uint8_t WhaleLiftoffYTbl[] = {
    0x20, 0x21, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
    0x28, 0x29, 0x2B, 0x2C, 0x2E, 0x30, 0x32, 0x34,
    0x36, 0x38, 0x3A, 0x3D, 0x3F, 0x42, 0x44, 0x47,
    0x4A, 0x4D, 0x50, 0x53, 0x57, 0x5A, 0x5E, 0x61,
    0x65, 0x69, 0x6C, 0x70, 0x75, 0x79, 0x7D, 0x81
};

// PITFALLS: None (Data table translation)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFA
//   entry_flags: auto
// CUSTOM_SPIKE: yes (Data table, no logic to spike)

// REVERSED_FUNCTION: field::WhaleLiftoffYTbl ($FA:F6)