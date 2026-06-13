#include "snes/snes.h"

// This "routine" is actually a lookup table (data), not executable code.
// It defines four 16-bit words used for button action mapping.
static const uint16_t BTN_ACTION_TABLE[] = {
    0x0000, 0x0080, 0x8000, 0x0040
};

// PITFALLS: None (Data table, not a routine)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  n/a
//   entry_flags: n/a