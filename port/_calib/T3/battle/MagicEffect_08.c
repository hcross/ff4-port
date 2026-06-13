#include "snes/snes.h"

// Sets bits 2-3 of $2706 (OR with $0C). Simple bit-set on a status byte.
// Entry mode: A 8-bit (mf=1), inherited from battle caller.
static void MagicEffect_08_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x2706] |= 0x0C;
}

// PITFALLS: none
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2706=1
//   output_ram:  0x2706=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::MagicEffect_08 ($D6:13)