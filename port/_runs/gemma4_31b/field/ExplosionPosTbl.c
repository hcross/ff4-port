#include "snes/snes.h"

// This is a lookup table defining explosion offsets/positions.
// The original assembly defines a sequence of 32 bytes.
static const uint8_t ExplosionPosTbl[] = {
    0x00, 0xC4, 0x17, 0xC9, 0x2A, 0xD6, 0x37, 0xE9, 
    0x3C, 0x00, 0x37, 0x17, 0x2A, 0x2A, 0x17, 0x37,
    0x00, 0x3C, 0xE9, 0x37, 0xD6, 0x2A, 0xC9, 0x17, 
    0xC4, 0x00, 0xC9, 0xE9, 0xD6, 0xD6, 0xE9, 0xC9
};

// PITFALLS: None. This is a static data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ExplosionPosTbl ($C9:EF)