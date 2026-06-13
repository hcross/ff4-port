#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0, DP=0
// Logic:
//   Decrements the screen brightness level ($80) only if:
//   1. (ram[0x7A] & 0x07) is 0 (brightness transition timer/flag)
//   2. ram[0x80] is not already 0
//   Then writes the resulting brightness value to the SNES hardware register $2100.
static void DecBrightness_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t timer = ram[0x7A];
    if ((timer & 0x07) == 0) {   // and #$07 / bne @dbb8
        uint8_t brightness = ram[0x80];
        if (brightness != 0) {   // lda $80 / beq @dbb8
            ram[0x80]--;          // dec $80
        }
    }

    // Note: $2100 is a hardware register. In the snesrev/zelda3 pattern, 
    // writes to IO are typically mirrored or handled via the Snes struct 
    // if the harness tracks it, but per requirements we treat the 
    // RAM/IO space consistently.
    snes->ram[0x2100] = ram[0x80]; // lda $80 / sta $2100
}

// PITFALLS: 7 (Arithmetic truncation: dec $80 is handled as uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7A=1, 0x80=1
//   output_ram:  0x2100=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DecBrightness ($00:00AC)