#include "snes/snes.h"

// This routine is a data table rather than executable code.
// It provides a sequence of 32 bytes used for mosaic effect transitions.
// Since it is a table, the "translation" is a constant array mapping.
static const uint8_t EventMosaicTbl[] = {
    0x0F, 0x1F, 0x2F, 0x3F, 0x4F, 0x5F, 0x6F, 0x7F, 0x8F, 0x9F, 0xAF, 0xBF, 0xCF, 0xDF, 0xEF, 0xFF,
    0xEF, 0xDF, 0xCF, 0xBF, 0xAF, 0x9F, 0x8F, 0x7F, 0x6F, 0x5F, 0x4F, 0x3F, 0x2F, 0x1F, 0x0F, 0x0F
};

// PITFALLS: None. This is a static data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none
//   CUSTOM_SPIKE: yes (Data table, not a function)

// REVERSED_FUNCTION: field::EventMosaicTbl ($FA:66)