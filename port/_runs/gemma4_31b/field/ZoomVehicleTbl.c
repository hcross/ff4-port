#include "snes/snes.h"

// This is not a routine, but a data table (constant array) used for 
// vehicle zoom/scaling factors.
// 
// Data mapping:
// @bfa8: 0x09, 0x13, 0x20
// @bfaB: 0x13, 0x2C, 0x42
// @bfaE: 0x08, 0x17, 0x1F
// @bfaB: 0x10, 0x21, 0x2C

static const uint8_t ZoomVehicleTbl[] = {
    0x09, 0x13, 0x20,
    0x13, 0x2C, 0x42,
    0x08, 0x17, 0x1F,
    0x10, 0x21, 0x2C
};

// PITFALLS: None (Data table)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none
//   CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::ZoomVehicleTbl ($BF:A8)