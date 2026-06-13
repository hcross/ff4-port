#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$86 (or 0), DP=0
// Logic:
//   Takes a 16-bit value in X, performs successive divisions by 10000, 1000, 100, and 10
//   to extract decimal digits.
//   Results are stored in RAM $180C-$1810 with an offset of 0x80 added to each digit.
//   This is a standard "Integer to ASCII/Decimal" conversion for a 5-digit display.
static void HexToDec_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // The routine uses DP=0 for its temporary variables
    // Initial value is passed in X
    uint16_t current_val = cpu->x;

    // --- Digit 1 (10000s) ---
    write16(ram, 0x26, current_val);      // stx $26
    write16(ram, 0x28, 10000);             // ldx #10000 / stx $28
    div16_emu(snes);                       // jsr Div16
    uint8_t digit = ram[0x2A];             // lda $2a (quotient)
    ram[0x180C] = (uint8_t)(digit + 0x80); // clc / adc #$80 / sta $180c
    current_val = read16(ram, 0x2C);       // ldx $2c (remainder)

    // --- Digit 2 (1000s) ---
    write16(ram, 0x26, current_val);       // stx $26
    write16(ram, 0x28, 1000);              // ldx #1000 / stx $28
    div16_emu(snes);                       // jsr Div16
    digit = ram[0x2A];                     // lda $2a
    ram[0x180D] = (uint8_t)(digit + 0x80); // clc / adc #$80 / sta $180d
    current_val = read16(ram, 0x2C);       // ldx $2c

    // --- Digit 3 (100s) ---
    write16(ram, 0x26, current_val);       // stx $26
    write16(ram, 0x28, 100);               // ldx #100 / stx $28
    div16_emu(snes);                       // jsr Div16
    digit = ram[0x2A];                     // lda $2a
    ram[0x180E] = (uint8_t)(digit + 0x80); // clc / adc #$80 / sta $180e
    current_val = read16(ram, 0x2C);       // ldx $2c

    // --- Digit 4 (10s) ---
    write16(ram, 0x26, current_val);       // stx $26
    write16(ram, 0x28, 10);                // ldx #10 / stx $28
    div16_emu(snes);                       // jsr Div16
    digit = ram[0x2A];                     // lda $2a
    ram[0x180F] = (uint8_t)(digit + 0x80); // clc / adc #$80 / sta $180f
    
    // --- Digit 5 (1s) ---
    uint8_t last_digit = ram[0x2C];        // lda $2c (remainder of last div)
    ram[0x1810] = (uint8_t)(last_digit + 0x80); // clc / adc #$80 / sta $1810
}

// PITFALLS: 7 (8-bit arithmetic: additive overflow dropped via uint8_t cast)
// HELPERS: div16_emu(snes) — delegates Div16 @ $84:07
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  0x180C=1, 0x180D=1, 0x180E=1, 0x180F=1, 0x1810=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x86
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::HexToDec ($86:BF)