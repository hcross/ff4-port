#include "snes/snes.h"

// Increments X and Y registers by 4.
// Used for advancing sprite index pointers in the field map.
static void NextSprite_c(Snes *snes) {
    // X and Y are treated as 16-bit registers in this context.
    snes->cpu->x += 4;
    snes->cpu->y += 4;
}

// PITFALLS: None. Simple register increment.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  x=16, y=16, a=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Outputs are in registers X/Y, not RAM)

// REVERSED_FUNCTION: field::NextSprite ($88:E2)