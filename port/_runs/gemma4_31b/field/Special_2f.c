#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C8, DP=0
// Sets the travel flag to 1 (travel to/from moon).
static void Special_2f_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xC3] = 1;
}

// PITFALLS: None (trivial assignment)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x00C3=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC8
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_2f ($C8:B8)