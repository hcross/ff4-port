#include "snes/snes.h"

// MapGfxBankTbl is a data table of 16 bytes, each representing the bank
// byte of a 24-bit pointer to a map graphics data block (MapGfx_0000..MapGfx_FFFF).
// It does not contain executable code and is used as a lookup table.
//
// Since this is a data block, not a routine, the translation is a direct
// C array of the same values. No execution logic required.
//
// Entry mode: N/A (data block)
// Entry: N/A
// Output: N/A

// PITFALLS: N/A (data table, no execution)
// HELPERS: N/A
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=auto, xf=auto, dp=auto, db=auto
//   entry_flags: z=auto, n=auto
static const uint8_t MapGfxBankTbl[16] = {
    0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1,
    0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1
};

// REVERSED_FUNCTION: field::MapGfxBankTbl ($B1:0004)