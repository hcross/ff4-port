#include "snes/snes.h"

// This routine initializes specific system/memory variables and loads 
// the whirlpool palette. It sets constants to various addresses 
// and clears three target memory locations.
static void _00cf98_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1704] = 0x06;
    ram[0x1705] = 0x03;
    ram[0x2C] = 0x58;
    ram[0x2E] = 0x60;

    load_whirlpool_pal_emu(snes);

    ram[0x79] = 0;
    ram[0x7A] = 0;
    ram[0x24] = 0;
}

// PITFALLS: 1 (DB=$00 is implicit for these absolute addresses in field module)
// HELPERS: load_whirlpool_pal_emu(snes) — delegates LoadWhirlpoolPal @ $D293
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1704=1, 0x1705=1, 0x2C=1, 0x2E=1, 0x79=1, 0x7A=1, 0x24=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00cf98 ($CF:98)