#include "snes/snes.h"

// This routine is a data table, not a function. It is referenced by other
// code via its address $E9:0022. The C equivalent is a const array.
//
// Entry mode: N/A (data only)
// No execution occurs here — this is pure data.

static const uint8_t CharRemoveTbl[21] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x81, 0x09, 0x81, 0x83, 0x0a, 0x80, 0x82,
    0x84, 0x0b, 0x0c, 0x80, 0x0d
};

// PITFALLS: none (data only)
// HELPERS: none (data only)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: field::CharRemoveTbl ($E9:0022)