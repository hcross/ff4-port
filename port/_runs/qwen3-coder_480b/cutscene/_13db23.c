#include "snes/snes.h"

// This function initializes four memory locations with fixed values:
//   $2080, $2082 = 0x0020
//   $20C0, $20C2 = 0x0040
// No inputs, no conditional branches, no subroutines.
static void _13db23_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    write16(ram, 0x2080, 0x0020);
    write16(ram, 0x2082, 0x0020);
    write16(ram, 0x20C0, 0x0040);
    write16(ram, 0x20C2, 0x0040);
}

// PITFALLS: None (no flags, no DB-sensitive ops, no shifts, no subroutines)
// HELPERS: write16 (for 16-bit LE storage)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=auto, xf=auto, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13db23 ($DB:23)