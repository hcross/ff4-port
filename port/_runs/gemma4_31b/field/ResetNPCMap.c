#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7F, DP=0
// Logic:
//   Zeroes out a block of 1024 bytes starting at $7F:4C00.
static void ResetNPCMap_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // The ASM uses absolute addressing $7f4c00,x.
    // In the snesrev/LakeSnes mapping, $7F:0000-$7F:FFFF is the upper 64KB of WRAM.
    // $7F:4C00 is offset 0x14C00 from the start of the 128KB WRAM array (0x7E0000).
    uint8_t *npc_map = &ram[0x14C00];

    for (uint16_t x = 0; x < 0x0400; x++) {
        npc_map[x] = 0;
    }
}

// PITFALLS: 1 (DB=$7F required for the memory range)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x14C00=1 (1024 bytes zeroed)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7F
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ResetNPCMap ($C2:ED)