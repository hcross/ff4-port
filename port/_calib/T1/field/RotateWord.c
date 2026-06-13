#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x8B (implicit for field), DP=0
// Logic:
//   This routine rotates a word (represented by $08 and $09 in DP) 
//   to the right. The input 'a' is shifted into the chain.
//   1. A is stored to $09 and $07.
//   2. ROR $09: bit 0 of $09 goes to Carry, Carry goes to bit 7 of $09.
//   3. ROR $08: bit 0 of $08 goes to Carry, Carry goes to bit 7 of $08.
//   4. ROR $07: bit 0 of $07 goes to Carry, Carry goes to bit 7 of $07.
//   5. The resulting byte at $08 is returned in A.
static uint16_t RotateWord_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // sta $09, sta $07
    ram[0x09] = (uint8_t)cpu->a;
    ram[0x07] = (uint8_t)cpu->a;

    // ror $09
    uint8_t val09 = ram[0x09];
    bool carry = (val09 & 0x01) != 0;
    val09 = (uint8_t)((val09 >> 1) | (cpu->c ? 0x80 : 0));
    cpu->c = carry;
    ram[0x09] = val09;

    // ror $08
    uint8_t val08 = ram[0x08];
    carry = (val08 & 0x01) != 0;
    val08 = (uint8_t)((val08 >> 1) | (cpu->c ? 0x80 : 0));
    cpu->c = carry;
    ram[0x08] = val08;

    // ror $07
    uint8_t val07 = ram[0x07];
    carry = (val07 & 0x01) != 0;
    val07 = (uint8_t)((val07 >> 1) | (cpu->c ? 0x80 : 0));
    cpu->c = carry;
    ram[0x07] = val07;

    // lda $08
    cpu->a = ram[0x08];
    
    // Update Z and N flags for the final LDA
    cpu->z = (cpu->a == 0);
    cpu->n = (cpu->a & 0x80) != 0;

    return cpu->a;
}

// PITFALLS: 7 (8-bit ROR truncation and Carry handling)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x08=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x8B
//   entry_flags: z=auto, n=auto, c=auto
//   CUSTOM_SPIKE: yes (returns result in A)

// REVERSED_FUNCTION: field::RotateWord ($8B:1D)