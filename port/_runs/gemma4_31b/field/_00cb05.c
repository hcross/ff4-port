#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// This routine initializes four entries by calculating an index based on
// the loop counter, calling setup routines _00df93 and _00dfc4.
//
// Logic:
//   For i = 0 to 3:
//     y = (i + 4) * 2
//     call _00df93(y)
//     ram[0x92] = 0
//     call _00dfc4()
//     i++
static void _00cb05_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x79] = 0; // stz $79

    for (uint8_t i = ram[0x79]; i < 4; i++) {
        // lda $79 / clc / adc #$04 / asl
        // Pitfall 7: wrap result to match 8-bit register truncation
        uint8_t a = (uint8_t)(i + 4); 
        a = (uint8_t)(a << 1);        

        snes->cpu->y = a;             // tay
        _00df93_emu(snes);           // jsr _00df93

        ram[0x92] = 0;                // stz $92
        _00dfc4_emu(snes);           // jsr _00dfc4

        ram[0x79] = i + 1;           // inc $79
    }
}

// PITFALLS: 7 (Arithmetic truncation: explicit (uint8_t) casts for ADC/ASL
// to ensure parity with 8-bit register overflow behavior)
// HELPERS: _00df93_emu(snes), _00dfc4_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x79=1, 0x92=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00cb05 ($CB:05)