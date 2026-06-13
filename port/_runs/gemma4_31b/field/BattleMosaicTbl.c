#include "snes/snes.h"

// This is not a functional routine but a static data table (lookup table).
// It defines a series of byte values used for Battle Mosaic mapping.
// In C, this is represented as a constant array.
static const uint8_t BattleMosaicTbl[] = {
    0x03, 0x23, 0x43, 0x63, 0x43, 0x23, 0x03, 0x23,
    0x43, 0x63, 0x43, 0x23, 0x03, 0x23, 0x43, 0x63,
    0x83, 0xA3, 0xC3, 0xE3, 0xF3
};

// PITFALLS: None. This is a data table, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none
// CUSTOM_SPIKE: yes (Data table; parity is checked via memory read at offset)

// REVERSED_FUNCTION: field::BattleMosaicTbl ($89:44)