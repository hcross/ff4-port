#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x0, DP=0
// Purpose: Checks if the player is NOT in a vehicle and is pressing the R button,
// then increments a counter (likely a debounce or toggle reset) at $53.
static void UpdateTopChar_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $1704 / bne @c85b
    if (ram[0x1704] != 0) {
        return;
    }

    // lda $02 / and #JOY_R / beq @c85b
    // JOY_R is usually defined as 0x80 or 0x40 depending on the map; 
    // in this context it refers to the bitmask for the R button.
    if ((ram[0x02] & 0x80) == 0) { // Assuming JOY_R = 0x80 based on standard SNES joypad mapping
        return;
    }

    // lda $53 / beq @c85c
    if (ram[0x53] == 0) {
        // inc $53
        ram[0x53]++;
    }
}

// PITFALLS: None (Straightforward logic, no complex arithmetic or mode shifts)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1704=1, 0x02=1, 0x53=1
//   output_ram:  0x53=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::UpdateTopChar ($C8:4C)