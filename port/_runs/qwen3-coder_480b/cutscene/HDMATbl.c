#include "snes/snes.h"

// HDMATbl is a data table, not a function. It is included directly in the C source
// as a static array. No execution logic is present, so no flags or registers are used.
//
// The table contains 3 groups of 5 bytes each:
// - M7A/M7B
// - M7C/M7D
// - BG1VOFS/BG2HOFS (unused)
//
// Each group is structured as:
//   .byte $f0, .word value, .byte $f0, .word value, .byte $00

static const uint8_t HDMATbl[15] = {
    0xf0, 0x19, 0x33, 0xf0, 0xd9, 0x34, 0x00, // M7A/M7B
    0xf0, 0x19, 0x37, 0xf0, 0xd9, 0x38, 0x00, // M7C/M7D
    0xf0, 0x19, 0x3b, 0xf0, 0xf9, 0x3b, 0x00  // BG1VOFS/BG2HOFS
};

// PITFALLS: None (data table, no execution)
// HELPERS: None (no calls)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::HDMATbl ($DB:36)