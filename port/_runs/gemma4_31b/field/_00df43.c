#include "snes/snes.h"

// This "routine" is actually a data table defining tank Y positions.
// It consists of 8 bytes of positions followed by 8 bytes of offsets/values.
// Layout: 4 pairs of 16-bit values.
static const uint16_t TANK_Y_POSITIONS[] = {
    0x0050, 0x0070, 0x0090, 0x00C0, // Y positions
    0x0008, 0x0018, 0x0008, 0x0018  // Y offsets/bounds
};

// PITFALLS: None. This is a data table, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none