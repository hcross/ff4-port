#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E7, DP=0
// This routine clears four consecutive bytes of character status data
// relative to the character index stored in X.
//
// Logic:
//   ram[0x1003 + x] = 0
//   ram[0x1004 + x] = 0
//   ram[0x1005 + x] = 0
//   ram[0x1006 + x] = 0
static void ClearCharStatus_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = snes->cpu->x;

    // DP=0, so $1003,x refers to ram[0x1003 + x]
    ram[0x1003 + x] = 0;
    ram[0x1004 + x] = 0;
    ram[0x1005 + x] = 0;
    ram[0x1006 + x] = 0;
}

// PITFALLS: 1 (DB=$E7 required for this module's WRAM mapping)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  0x1003=1, 0x1004=1, 0x1005=1, 0x1006=1 (offset by X)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE7
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ClearCharStatus ($E7:C6)