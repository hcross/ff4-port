#include "snes/snes.h"

// This is a data table, not a routine. It's an array of 13 16-bit little-endian values.
// No execution logic to translate — the label SoloBattleTbl is just a pointer to data.
// The array contents are:
// [0] = 0x00F7, [1] = 0x00F8, [2] = 0x00F9, [3] = 0x00F1, [4] = 0x00EE,
// [5] = 0x00F6, [6] = 0x00EF, [7] = 0x00F0, [8] = 0x00F3, [9] = 0x00FD,
// [10] = 0x01A8, [11] = 0x01B3, [12] = 0x01B4

// PITFALLS: none (data only)
// HELPERS: none (data only)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=auto, xf=auto, dp=auto, db=auto
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::SoloBattleTbl ($E7:009E)