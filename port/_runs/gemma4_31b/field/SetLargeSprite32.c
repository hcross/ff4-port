#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 8-bit (xf=1), DB=$E1, DP=0
// This routine initializes a 32-bit-aligned sprite area (or similar) by
// writing the byte 0xAA (%10101010) to 8 consecutive bytes starting at $0500.
static void SetLargeSprite32_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t val = 0xAA;

    for (uint8_t x = 0; x < 8; x++) { // ldx #0 / cpx #8 / bne loop
        ram[0x0500 + x] = val;       // sta $0500,x
    }
}

// PITFALLS: None (Simple loop, no arithmetic or flag dependencies on entry)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0500=1, 0x0501=1, 0x0502=1, 0x0503=1, 0x0504=1, 0x0505=1, 0x0506=1, 0x0507=1
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0xE1
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::SetLargeSprite32 ($E1:C0)