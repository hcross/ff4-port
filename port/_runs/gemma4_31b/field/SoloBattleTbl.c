#include "snes/snes.h"

// This routine is not a functional routine but a data table (lookup table).
// In the snesrev/zelda3 pattern, data tables are translated as 
// static constant arrays for the C reimplementation to reference.
// The ASM .word directives indicate a sequence of 16-bit little-endian values.

static const uint16_t SoloBattleTbl[] = {
    0x00F7, 0x00F8, 0x00F9, 0x00F1, 0x00EE, 0x00F6, 0x00EF, 0x00F0,
    0x00F3, 0x00FD, 0x01A8, 0x01B3, 0x01B4
};

// PITFALLS: None. This is a data table, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: auto
// CUSTOM_SPIKE: yes (Data table validation requires checking access from callers)

// REVERSED_FUNCTION: field::SoloBattleTbl ($E7:9E)