#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0xDA, DP=0
// Purpose: Calculate a sine-based offset and update coordinate/buffer $22 and $20.
// Logic:
//   1. Call CalcSine (result in Y).
//   2. Transition A to 16-bit, load Y, divide by 4, add 0x0070, store in $22.
//   3. Transition A to 8-bit, load $20, multiply by 4, store in Y.
//   4. Load $22 into A (8-bit) and return.
static void _00da79_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // jsr CalcSine
    CalcSine_emu(snes); 
    
    // longa / tya / lsr2
    // Y is 16-bit (xf=0). result of CalcSine is in Y.
    uint16_t a16 = cpu->y;
    a16 >>= 2; // lsr2 (Shift right twice)

    // clc / adc #$0070 / sta $22
    uint16_t res16 = a16 + 0x0070;
    write16(ram, 0x22, res16);

    // lda #0 / shorta
    // (lda #0 is a dummy/clear here, followed by mode switch)

    // lda $20 / asl2 / tay
    uint8_t a8 = ram[0x20];
    a8 = (uint8_t)(a8 << 2); // asl2 (Pitfall 7: truncate to 8-bit)
    cpu->y = (uint16_t)a8;

    // lda $22 / rts
    cpu->a = ram[0x22];
}

// PITFALLS: 6 (Mode A switching 16<->8), 7 (asl2 8-bit truncation)
// HELPERS: CalcSine_emu(snes) — delegates CalcSine @ $E79A
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x20=1
//   output_ram:  0x22=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDA
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00da79 ($DA:79)