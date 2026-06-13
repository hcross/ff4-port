#include "snes/snes.h"

// MoonTileTbl is a raw data table, not a routine. Emitting as a C array.
// No execution logic to translate. This is a data label.
static const uint8_t MoonTileTbl_c[32] = {
    4,3,2,1,0,1,2,3,4,5,6,7,7,7,6,5,
    7,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7
};

// PITFALLS: None (data table, no execution)
// HELPERS: None (no calls)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none
// REVERSED_FUNCTION: cutscene::MoonTileTbl ($E2:AC)