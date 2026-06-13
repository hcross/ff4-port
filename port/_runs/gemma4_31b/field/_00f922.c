#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$F9, DP=0
// This routine calculates character-related coordinates or indices
// based on values in $43 and $44, outputting results to $9D and $9E.
static void _00f922_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x99] = 0;
    uint8_t val44 = ram[0x44];
    uint8_t tmp9a = (uint8_t)(val44 & 0x0F);
    ram[0x9A] = tmp9a;

    if ((val44 & 0x10) != 0) {
        tmp9a = (uint8_t)(tmp9a + 0x20); // Pitfall 7
        ram[0x9A] = tmp9a;
    }

    // Sequence of 2x (LSR $9A / ROR $99)
    // LSR $9A: bit 0 -> Carry, shift right
    // ROR $99: Carry -> bit 0, shift right (Wait, ROR is Rotate Right)
    // 65816 ROR: Carry is shifted into bit 7, bit 0 shifted into Carry.
    uint8_t v9a = ram[0x9A];
    uint8_t v99 = ram[0x99];
    uint8_t c;

    c = v9a & 1; v9a >>= 1;
    v99 = (uint8_t)((v99 >> 1) | (c << 7));

    c = v9a & 1; v9a >>= 1;
    v99 = (uint8_t)((v99 >> 1) | (c << 7));

    ram[0x9A] = v9a;
    ram[0x99] = v99;

    uint8_t val43 = (uint8_t)(ram[0x43] & 0x1F);
    val43 = (uint8_t)(val43 << 1); // Pitfall 7: ASL
    ram[0x43] = val43;

    if ((val43 & 0x20) != 0) {
        ram[0x9A] = (uint8_t)(ram[0x9A] + 0x04); // Pitfall 7: ADC
    }

    ram[0x99] = (uint8_t)((ram[0x43] & 0x1F) + ram[0x99]); // Pitfall 7: ADC
    ram[0x9A] = (uint8_t)(ram[0x9A] + 0x30); // Pitfall 7: ADC
    ram[0x9D] = (uint8_t)(ram[0x99] + 0x20); // Pitfall 7: ADC
    ram[0x9E] = (uint8_t)(ram[0x9A] + 0x00); // Pitfall 7: ADC

    ram[0x94]++;
}

// PITFALLS: 7 (All ADC, ASL, and shifts truncated to 8-bit via explicit uint8_t casts)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x43=1, 0x44=1
//   output_ram:  0x9D=1, 0x9E=1, 0x94=1, 0x99=1, 0x9A=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xF9
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00f922 ($F9:22)