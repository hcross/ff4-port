#include "snes/snes.h"

// SpellListTbl is a ROM data table at $FD:DD, not a function.
// It contains 16 entries of 3 bytes each (48 bytes total).
// The table is used by spell list lookup code.
// This C function returns a pointer to the table for use by other translated routines.
static const uint8_t *SpellListTbl_c(void) {
    static const uint8_t table[48] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x02, 0x03, 0x04,
        0x05, 0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0x08, 0xFF, 0x09, 0xFF, 0xFF,
        0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x04,
        0xFF, 0x0C, 0xFF, 0x0A, 0x0B, 0xFF, 0x0A, 0x0B, 0xFF,
        0xFF, 0xFF, 0xFF
    };
    return table;
}

// PITFALLS: none (data table, no code)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none
//   CUSTOM_SPIKE: yes (data table, not a routine; parity harness should skip)
// REVERSED_FUNCTION: battle::SpellListTbl ($FD:DD) [data table]