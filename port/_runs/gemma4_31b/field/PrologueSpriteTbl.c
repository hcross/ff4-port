#include "snes/snes.h"

// This routine is not a functional block of code but a data table (lookup table).
// In the context of a C reimplementation, this is represented as a constant array.
// The data consists of 4-byte entries (likely mapping sprite types/IDs to 
// coordinate/attribute parameters).
static const uint8_t PROLOGUE_SPRITE_TBL[] = {
    0x40, 0x68, 0x00, 0x30,
    0x48, 0x68, 0x01, 0x30,
    0x40, 0x70, 0x02, 0x30,
    0x48, 0x70, 0x03, 0x30
};

// To maintain parity with the ROM's memory map for the harness, 
// this data is conceptually located at $FA:56.
// If the C engine needs to access this via a pointer, it would be:
// uint8_t *tbl = &snes->rom[0xFA0000 + 0x56]; // (assuming bank mapping)

// PITFALLS: None (Data table, no execution logic)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: auto
// REVERSED_FUNCTION: field::PrologueSpriteTbl ($FA:56)