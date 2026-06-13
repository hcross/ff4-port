#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), DB=$00 (Direct Page access), DP=0
// Logic:
// This routine rotates a sequence of bytes in memory using the carry flag.
// It seeds $09 and $07 with the current value of A, then performs a chain of 
// right-rotates (ROR) through $09, $08, and $07. The result of $08 is returned in A.
static void RotateWord_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // sta $09
    // sta $07
    ram[0x09] = (uint8_t)cpu->a;
    ram[0x07] = (uint8_t)cpu->a;

    // ror $09
    uint8_t val09 = ram[0x09];
    bool c09 = cpu->c;
    cpu->c = (val09 & 0x01) != 0;
    ram[0x09] = (uint8_t)((val09 >> 1) | (c09 << 7));

    // ror $08
    uint8_t val08 = ram[0x08];
    bool c08 = cpu->c;
    cpu->c = (val08 & 0x01) != 0;
    ram[0x08] = (uint8_t)((val08 >> 1) | (c08 << 7));

    // ror $07
    uint8_t val07 = ram[0x07];
    bool c07 = cpu->c;
    cpu->c = (val07 & 0x01) != 0;
    ram[0x07] = (uint8_t)((val07 >> 1) | (c07 << 7));

    // lda $08
    cpu->a = ram[0x08];
    cpu->z = (cpu->a == 0);
    cpu->n = (cpu->a & 0x80) != 0;
}

// PITFALLS: 7 (Shift truncation: explicitly cast to uint8_t to simulate 8-bit ROR)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x08=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto, c=auto
// CUSTOM_SPIKE: no
// REVERSED_FUNCTION: field::RotateWord ($8B:1D)