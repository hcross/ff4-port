#include "snes/snes.h"

// This "routine" is actually a data table (lookup table) located at $F3:A1.
// In a C reimplementation, this is translated as a static constant array.
// Based on the ASM source, it is a sequence of 53 bytes.
static const uint8_t TABLE_14F3A1[] = {
    0x00, 0x02, 0x04, 0x06, 0x09, 0x0C, 0x0F, 0x12, 0x16, 0x19, 0x1C, 0x1F, 0x23, 0x27, 0x2A, 0x2D,
    0x31, 0x35, 0x39, 0x3D, 0x41, 0x46, 0x4A, 0x4F, 0x54, 0x59, 0x5E, 0x63, 0x68, 0x6D, 0x73, 0x79,
    0x7F, 0x86, 0x8C, 0x93, 0x9A, 0xA2, 0xAA, 0xB2, 0xBB, 0xC4, 0xCD, 0xD8, 0xE4
};

// Since this is a data block and not an executable routine, the parity harness
// will validate access to this memory region via the emulator or direct 
// pointer comparison in the field module.

// PITFALLS: None. This is a data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram: none
//   entry_mode:  none
//   entry_flags: none

// REVERSED_FUNCTION: field::_14f3a1 ($F3:A1)