#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$F4, DP=0
// Logic: 
//   1. Calculates a temporary offset based on A shifted right 3 times.
//   2. Modifies that offset if certain conditions in RAM are met ($0FE5 is set or $1701 is non-zero).
//   3. Returns (A & 0x07) in the Y register.
// Note: The sequence at @f414-@f426 modifies RAM $3D/$3E, but these are essentially 
// scratch variables. The final result only depends on the original A and the 
// mask applied at @f42d.
static void GetNPCSwitchOffset_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    // pha / lsr3 / sta $3d / stz $3e
    uint8_t temp = a >> 3; // Pitfall 7: truncate/shift 8-bit
    ram[0x3D] = temp;
    ram[0x3E] = 0;

    // lda $0fe5 / bmi @f426
    uint8_t val_0fe5 = ram[0x0FE5];
    bool condition_met = (val_0fe5 & 0x80) != 0; // bmi (branch if minus/negative)

    if (!condition_met) {
        // lda $1701 / beq @f42d
        uint8_t val_1701 = ram[0x1701];
        if (val_1701 != 0) {
            condition_met = true; // beq @f42d not taken
        }
    }

    if (condition_met) {
        // @f426: lda $3d / clc / adc #$20 / sta $3d
        ram[0x3D] = (uint8_t)(ram[0x3D] + 0x20); // Pitfall 7: 8-bit wrap
    }

    // @f42d: pla / and #$07 / tay
    uint8_t result = a & 0x07;
    snes->cpu->y = result;
}

// PITFALLS: 7 (8-bit shift and addition truncation)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x0FE5=1, 0x1701=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xF4
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (result is in register Y, not RAM)

// REVERSED_FUNCTION: field::GetNPCSwitchOffset ($F4:14)