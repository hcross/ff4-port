#include "snes/snes.h"

// This function initializes four memory locations with fixed values:
//   $2080, $2082 = 8
//   $20C0, $20C2 = 16
// No inputs, no outputs in registers. All operations are 16-bit.
static void _13db10_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    write16(ram, 0x2080, 0x0008);
    write16(ram, 0x2082, 0x0008);
    write16(ram, 0x20C0, 0x0010);
    write16(ram, 0x20C2, 0x0010);
}

// PITFALLS: 6 (A 16-bit mode assumed due to 16-bit literals and stores)
// HELPERS: write16 (for 16-bit memory writes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13db10 ($DB:10)