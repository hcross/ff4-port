#include "snes/snes.h"

// CharAddTbl is a raw data table, not a routine. No execution logic to translate.
// This is a 20-byte lookup table used by field code.
// No C function body needed — the data is embedded in the ROM and accessed via address.

// PITFALLS: none (data table, no execution)
// HELPERS: none (no calls)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::CharAddTbl ($E9:A5)