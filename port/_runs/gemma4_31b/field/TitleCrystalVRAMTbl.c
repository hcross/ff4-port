#include "snes/snes.h"

// This routine is not an executable function but a data table
// containing 10 VRAM address offsets used for Title Crystal animations.
// The "translation" is a constant array mapping the source words.
static const uint16_t TitleCrystalVRAMTbl_c[] = {
    0x19CE, 0x19EE, 0x1A0E, 0x1A2E, 0x1A4E, 0x1A6E, 0x1A8E, 0x1AAE,
    0x1ACE, 0x1AEE
};

// Since this is a data table and not a routine, we provide a helper 
// to access it if the parity harness requires a functional interface 
// to simulate the original memory access at $88:CE.
static uint16_t get_title_crystal_vram_tbl(Snes *snes, int index) {
    // Original address is $88:CE. 
    // The table is stored in the program bank (K=$88).
    // In the native reimplementation, we use the C array.
    if (index < 0 || index >= 10) return 0;
    return TitleCrystalVRAMTbl_c[index];
}

// PITFALLS: None. This is a static data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: auto
// REVERSED_FUNCTION: field::TitleCrystalVRAMTbl ($88:CE)