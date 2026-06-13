#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x00, DP=0x00
// Logic:
//   Generate a random number. If it is < 0x80, apply an offset of 8
//   to two 16-bit values stored at $5C and $60 and write them to
//   hardware registers (or mapped RAM) at $210E and $2110.
static void _00edf6_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint16_t rand_val = rand_emu(snes); // jsr Rand (delegated)
    uint8_t a = (uint8_t)rand_val;

    if (a >= 0x80) {                    // cmp #$80 / bcs @ee1b
        return;
    }

    // First 16-bit addition: ram[$5C] + 8
    uint16_t val1 = read16(ram, 0x5C);
    val1 += 8;                          // clc / adc #$08 (lo) / adc #$00 (hi)
    write16(ram, 0x210E, val1);         // sta $210e / sta $210e (overwritten by high byte)

    // Second 16-bit addition: ram[$60] + 8
    uint16_t val2 = read16(ram, 0x60);
    val2 += 8;                          // clc / adc #$08 (lo) / adc #$00 (hi)
    write16(ram, 0x2110, val2);         // sta $2110 / sta $2110 (overwritten by high byte)
}

// PITFALLS: 7 (Arithmetic truncation handled by using uint16_t for 
// the 16-bit chain of ADC operations), 6 (Mode A is 8-bit based on 
// the use of #$80 and the 8-bit sequence of LDA/ADC/STA)
// HELPERS: rand_emu(snes) — delegates Rand @ $8593
//          read16/write16 — little-endian 16-bit accessors
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x5C=2, 0x60=2
//   output_ram:  0x210E=2, 0x2110=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00edf6 ($ED:F6)