#include "snes/snes.h"

// This is not a routine, but a data table.
// In a C reimplementation, this is represented as a constant array.
// The original assembly contains 5 bytes: 1, 3, 0, 4, 2.
static const uint8_t CharOrderTbl[] = {
    1, 3, 0, 4, 2
};

// PITFALLS: None. This is a static data table, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none

// REVERSED_FUNCTION: menu::CharOrderTbl ($A1:C8)