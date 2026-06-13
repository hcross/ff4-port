#include "snes/snes.h"

// This "routine" is actually a data table (bankbytes) and not executable code.
// It defines a lookup table of 16 bank values used for MapGFX mapping.
// In the C native reimplementation, this is represented as a constant array.
static const uint8_t MapGfxBankTbl[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

// PITFALLS: None. This is a data definition, not a logic routine.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: auto