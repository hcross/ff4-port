#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine clears a specific 16KB region of the BG tilemap.
// It assumes the accumulator (A) already contains the value to be written 
// (typically 0, though not explicitly cleared in this routine).
static void ClearBGTilemap_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t val = (uint8_t)snes->cpu->a;

    // The ASM uses absolute addressing $7f5c71 + X
    // $7f5c71 is outside the standard 128KB WRAM ($7E0000-$7FFFFF).
    // However, based on the project's snesrev pattern, these addresses 
    // map to the emulator's RAM array.
    // $7f5c71 maps to offset 0x5C71 relative to the $7F bank.
    // In snes->ram (128KB total), $7E is 0x0000 and $7F is 0x10000.
    // $7F5C71 = 0x10000 + 0x5C71 = 0x15C71.

    for (uint16_t x = 0; x < 0x4000; x++) {
        ram[0x15C71 + x] = val;
    }
}

// PITFALLS: 1 (Address $7f5c71 requires correct bank mapping to snes->ram)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x15C71=1 (Range: 0x15C71 to 0x19C70)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7F
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ClearBGTilemap ($F6:25)