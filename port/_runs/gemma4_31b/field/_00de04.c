#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DE, DP=0
// Logic:
//   Initializes specific global state variables, likely for airship movement/animation.
//   $1704 = 4, $B7 = 16, $06FD = 15 (Airship speed), $AC = 3, $AD = 32.
static void _00de04_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1704] = 0x04;
    ram[0x00B7] = 0x10;
    ram[0x06FD] = 0x0F; // set airship animation speed
    ram[0x00AC] = 0x03;
    ram[0x00AD] = 0x20;
}

// PITFALLS: None (simple linear assignment)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1704=1, 0x00B7=1, 0x06FD=1, 0x00AC=1, 0x00AD=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDE
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00de04 ($DE:04)