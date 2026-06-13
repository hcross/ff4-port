#include "snes/snes.h"

// Data table containing tile indices for the Gil window.
// This is a constant byte array, not an executable routine.
static const uint8_t GilWindowTiles1_data[] = {
    0x16, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 
    0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20,
    0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x18, 0x20
};

// PITFALLS: None. This is a static data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags:  none
// REVERSED_FUNCTION: field::GilWindowTiles1 ($F5:F6)