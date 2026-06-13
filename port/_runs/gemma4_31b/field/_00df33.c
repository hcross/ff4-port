#include "snes/snes.h"

// This is a data table, not an executable routine.
// It contains 16 bytes of coordinate data (tank x positions).
// The data is organized as 8 pairs of bytes.
static const uint8_t tank_x_positions[] = {
    0xf8, 0xff, 0xd0, 0xff, 0xf8, 0xff, 0xd0, 0xff,
    0x20, 0x00, 0x32, 0x00, 0x44, 0x00, 0x56, 0x00
};

// PITFALLS: None (Data block)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none