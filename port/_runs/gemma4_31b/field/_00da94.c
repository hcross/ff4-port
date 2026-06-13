#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 8-bit (xf=1), DB=0x00, DP=0
// This routine initializes a memory block starting at 0x0500 with a
// specific bit pattern (0xAA) for 8 bytes.
static void _00da94_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t val = 0xAA; // lda #%10101010

    for (uint8_t x = 0; x < 8; x++) { // ldx #0 / inx / cpx #8 / bne
        ram[0x0500 + x] = val;       // sta $0500,x
    }
}

// PITFALLS: None. (Simple loop, no complex flags or mode pollution).
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0500=1, 0x0501=1, 0x0502=1, 0x0503=1, 0x0504=1, 0x0505=1, 0x0506=1, 0x0507=1
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00da94 ($DA:94)