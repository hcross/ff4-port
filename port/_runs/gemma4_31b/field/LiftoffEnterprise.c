#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   Initializes counters $7a and $b7 to 0.
//   Loop 1: Calls Animate4, checks if $79 reaches $20.
//     If < $20: shifts $79 right, writes result to $06FD (airship speed), increments $79 and loops.
//   Loop 2: Once $79 >= $20, increments $b7, sets $ad = $b7 + 0x10, calls UpdateZoomPal with $b7.
//     Increments $79 until it reaches $30, then returns.
static void LiftoffEnterprise_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x7A] = 0;
    ram[0xB7] = 0;

loop_a259:;
    animate4_emu(snes); // jsr Animate4

    uint8_t val79 = ram[0x79];
    if (val79 < 0x20) { // cmp #$20 / bcs @a26b (inverted)
        uint8_t shifted = (uint8_t)(val79 >> 1); // lsr A (Pitfall 7)
        ram[0x06FD] = shifted;
        ram[0x79] = val79 + 1; // inc $79
        goto loop_a259;
    }

// @a26b:
    ram[0xB7]++; // inc $b7
    uint8_t b7 = ram[0xB7];
    ram[0xAD] = (uint8_t)(b7 + 0x10); // clc / adc #$10 / sta $ad
    
    // UpdateZoomPal call: expected input is A = ram[$B7]
    snes->cpu->a = b7;
    update_zoom_pal_emu(snes); // jsl UpdateZoomPal

    ram[0x79]++; // inc $79
    if (ram[0x79] != 0x30) { // cmp #$30 / bne @a259
        goto loop_a259;
    }
}

// PITFALLS: 7 (lsr A in 8-bit mode truncated to uint8_t)
// HELPERS: animate4_emu(snes), update_zoom_pal_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x79=1
//   output_ram:  0x06FD=1, 0x7A=1, 0xB7=1, 0xAD=1, 0x79=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LiftoffEnterprise ($A2:55)