#include "snes/snes.h"

// MagicEffect_08 — Sets bits 2 and 3 of the battle byte at $2706.
static void MagicEffect_08_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x2706] |= 0x0C;
}

// PITFALLS: 6 (assumed 8-bit A; battle default mf=true prevents 16-bit sta from corrupting $2707)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2706=1
//   output_ram:  0x2706=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::MagicEffect_08 ($D6:13)