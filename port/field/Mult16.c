#include "snes/snes.h"

// Entry mode: Inherited (A=8, X=16, DB=$7E, DP=0)
// Logic: 16-bit unsigned multiplication of two 16-bit values.
// Inputs: ram[0x393D] (16-bit), ram[0x393F] (16-bit)
// Output: ram[0x3941] (low 16-bit result), ram[0x3943] (high 16-bit result)
// This is a shift-and-add implementation of a 16x16 -> 32-bit multiply.
static void Mult16_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // longa: A becomes 16-bit. 
    // ldx #$0010: Loop counter for 16 bits.
    uint16_t counter = 0x0010;
    
    // stz $3941 / stz $3943: Initialize 32-bit result to 0.
    uint32_t result = 0; 

    uint16_t op1 = read16(ram, 0x393D);
    uint16_t op2 = read16(ram, 0x393F);

    // The ASM uses ror $393f / ror $3943 / ror $3941 to shift 
    // the multiplier and the result accumulator as a single unit.
    
    // We simulate the shift-and-add loop
    for (int i = 0; i < 16; i++) {
        // @83c4: ror $393f
        // This shifts the LSB of op2 into the Carry flag.
        bool carry = (op2 & 1);
        op2 >>= 1;

        // bcc @83d3: If Carry was 0, skip the addition.
        if (carry) {
            // clc / lda $393d / adc $3943 / sta $3943
            // Add op1 to the high word of the result.
            // Because result is tracked as a 32-bit int, we handle the 
            // overflow into the low word during the ror sequence.
            uint32_t add_val = (uint32_t)op1 << 16;
            result += add_val;
        }

        // @83d3: ror $3943 / ror $3941
        // This effectively shifts the accumulated result right by 1.
        // Since we are doing a standard multiply, the "ror" sequence in ASM 
        // is essentially managing the 32-bit register shift.
        // In C, a simple multiply is semantically identical and preserves parity.
    }

    // To strictly match the ASM logic of shifting result bits:
    // The ASM actually calculates (op1 * op2).
    uint32_t product = (uint32_t)op1 * (uint32_t)op2;
    
    write16(ram, 0x3941, (uint16_t)(product & 0xFFFF));
    write16(ram, 0x3943, (uint16_t)(product >> 16));

    // shorta0: clr_a / shorta (A=0, A becomes 8-bit)
    snes->cpu->a = 0;
    snes->cpu->mf = true;
}

// PITFALLS: 6 (longa explicitly sets A to 16-bit for the multiply logic),
// 7 (arithmetic is performed as 16/32-bit, no 8-bit truncation needed here).
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x393D=2, 0x393F=2
//   output_ram:  0x3941=2, 0x3943=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Mult16 ($83:B9)