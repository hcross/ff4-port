#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$A1 (Field), DP=0
// Purpose: Sets the active object to 'ship' and sets the movement speed to 1.
static void BoardShip_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1704] = 0x07; // Set object to ship
    ram[0x00AC] = 0x01; // Set movement speed to 1
}

// PITFALLS: None (Straightforward assignments)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1704=1, 0x00AC=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xA1
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::BoardShip ($A1:5E)