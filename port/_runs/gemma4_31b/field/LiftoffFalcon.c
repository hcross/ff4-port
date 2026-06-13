#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine handles the airship (Falcon) takeoff sequence:
// 1. Resets counters at $7A and $B8.
// 2. Loops until $79 reaches 0x20, calling Animate4 and updating speed at $06FD.
// 3. Then increments $B8 and calls UpdateZoomPal until $79 reaches 0x30.
static void LiftoffFalcon_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x7A] = 0;
    ram[0xB8] = 0;

loop_a2c1:;
    animate4_emu(snes); // jsr Animate4

    uint8_t val79 = ram[0x79];
    if (val79 >= 0x20) { // cmp #$20 / bcs @a2d3
        goto phase_zoom;
    }

    // Pitfall 7: lsr A in 8-bit mode (truncate to 8-bit)
    uint8_t speed = (uint8_t)(val79 >> 1);
    ram[0x06FD] = speed; // sta $06fd
    ram[0x79]++;         // inc $79
    goto loop_a2c1;     // jmp @a2c1

phase_zoom:;
    ram[0xB8]++;         // inc $b8
    uint8_t b8 = ram[0xB8];
    
    // Pitfall 7: adc #$10 in 8-bit mode (truncate to 8-bit)
    ram[0xAD] = (uint8_t)(b8 + 0x10); // clc / adc #$10 / sta $ad
    
    // UpdateZoomPal expects A = ram[$B8]
    snes->cpu->a = b8; 
    update_zoom_pal_emu(snes); // jsl UpdateZoomPal

    ram[0x79]++;         // inc $79
    if (ram[0x79] != 0x30) { // cmp #$30 / bne @a2c1
        goto loop_a2c1;
    }
}

// PITFALLS: 7 (8-bit arithmetic/shifts for LSR and ADC)
// HELPERS: animate4_emu(snes), update_zoom_pal_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x79=1
//   output_ram:  0x7A=1, 0xB8=1, 0xAD=1, 0x06FD=1, 0x79=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LiftoffFalcon ($A2:BD)