#include "snes/snes.h"

// This routine is not a functional block of code but a data table
// containing tile indices used for rendering the bottom part of the map title.
// In the original ASM, it is defined as a sequence of bytes.
// In a native C implementation, this would be treated as a constant array
// and accessed via pointer/offset from the caller.
static const uint8_t MapTitleTilesBtm_data[] = {
    0x1B, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20,
    0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20,
    0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20,
    0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1D, 0x20
};

// Note: Since this "routine" consists only of .byte directives, it does not
// contain executable instructions. The caller typically loads the address 
// of this table into a register (e.g., X or Y) and iterates through the bytes.
// For the purpose of the parity harness and translation, this is represented
// as a data constant.

// PITFALLS: None (Data table, no CPU state manipulation)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: auto
// REVERSED_FUNCTION: field::MapTitleTilesBtm ($F7:B6)