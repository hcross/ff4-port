#include "snes/snes.h"

// Constrains the accumulator to a maximum of 99.
// If A >= 99, sets A = 99. Otherwise leaves A unchanged.
static void Max99_c(Snes *snes, uint8_t a) {
    if (a >= 99) {
        snes->cpu->a = 99;
    } else {
        snes->cpu->a = a;
    }
}

// PITFALLS: None
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::Max99 ($9E:20)