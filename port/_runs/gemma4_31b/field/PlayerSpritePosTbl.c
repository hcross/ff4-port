#include "snes/snes.h"

// The original ASM routine is actually a data table (constant bytes),
// not executable code. In the C reimplementation, this is represented
// as a static constant array.
//
// The table contains 4 entries of 4 bytes each, likely representing
// X/Y offsets or coordinate pairs for player sprites.
static const uint8_t PlayerSpritePosTbl[] = {
    0x70, 0x6D, 0x00, 0x00, // Entry 0
    0x78, 0x6D, 0x00, 0x00, // Entry 1
    0x70, 0x75, 0x00, 0x00, // Entry 2
    0x78, 0x75, 0x00, 0x00  // Entry 3
};

// PITFALLS: None (Data table, not a routine)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: auto

// REVERSED_FUNCTION: field::PlayerSpritePosTbl ($C0:B4)