#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x00, DP=0
// Logic:
//   Initializes counters at $79 and $b5 to 0.
//   Loops 16 times:
//     - Calls Animate4
//     - Increments counters
//   Loop terminates when ram[0x79] reaches 0x10.
static void LiftoffBkChoco_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x79] = 0; // stz $79
    ram[0xB5] = 0; // stz $b5

    do {
        animate4_emu(snes);    // jsr Animate4
        ram[0xB5]++;            // inc $b5
        ram[0x79]++;            // inc $79
    } while (ram[0x79] != 0x10); // lda $79 / cmp #$10 / bne @a06d
}

// PITFALLS: None relevant for this routine (simple loop, no complex flags or 16-bit shifts).
// HELPERS: animate4_emu(snes) — delegates Animate4 (@a014)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x79=1, 0xB5=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LiftoffBkChoco ($A0:69)