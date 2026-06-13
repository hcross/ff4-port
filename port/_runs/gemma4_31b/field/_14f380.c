#include "snes/snes.h"

// This routine is not a function, but a data table.
// It provides a sequence of 49 values (offsets or thresholds).
// Since the target is a data table, it is represented as a static const array.
static const uint8_t table_14f380[] = {
    0x00, 0x03, 0x07, 0x0a, 0x0f, 0x13, 0x17, 0x1b, 0x20, 0x25, 0x2a, 0x2f, 0x34, 0x39, 0x3e, 0x44,
    0x4a, 0x50, 0x56, 0x5d, 0x64, 0x6b, 0x72, 0x7a, 0x82, 0x8b, 0x94, 0x9d, 0xa8, 0xb3, 0xbf, 0xcc,
    0xda
};

// Note: The provided ASM source lists 49 bytes.
// The requested translation is for the data residing at $F3:80.
// Because this is a data block and not an executable routine, 
// no C function body is produced.

// PITFALLS: None (Data table)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: auto

// REVERSED_FUNCTION: field::_14f380 ($F3:80)