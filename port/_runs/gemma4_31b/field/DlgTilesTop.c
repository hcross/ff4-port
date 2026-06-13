#include "snes/snes.h"

/**
 * DlgTilesTop is a data table rather than an executable routine.
 * It contains a sequence of tile indices used for the top border 
 * of dialogue boxes.
 * 
 * Note: In a C reimplementation targeting a native environment, 
 * this data would be stored as a constant array. Since the parity 
 * harness treats the ROM as a byte array, we represent this as 
 * a pointer to the ROM data or a static array.
 */
static const uint8_t DlgTilesTop_data[] = {
    0x00, 0x20, 0x16, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20,
    0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20,
    0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20,
    0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x17, 0x20, 0x18, 0x20, 0x00, 0x20
};

// This "function" is actually a data block. In the parity harness,
// the system simply reads from this ROM address.
static void DlgTilesTop_c(Snes *snes) {
    // No-op: This is a data table. 
    // The emulator/harness accesses this via ROM read at $F6:D6.
}

// PITFALLS: None (Data table, not executable code)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: auto
// REVERSED_FUNCTION: field::DlgTilesTop ($F6:D6)