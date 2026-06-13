#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), DB and DP inherited from caller
// Rotates a 24-bit value across $07/$08/$09 right through carry.
// Input: A = value stored to $09 (MSB) and $07 (LSB), carry = input carry
//        $08 = middle byte (pre-existing)
// Output: A = rotated $08, carry = bit 0 of original A
//         $07, $08, $09 all updated
static uint8_t RotateWord_c(Snes *snes, uint8_t a_in) {
    uint8_t *ram = snes->ram;
    bool carry = snes->cpu->c;
    
    ram[0x09] = a_in;
    ram[0x07] = a_in;
    
    // ror $09
    bool new_carry = ram[0x09] & 1;
    ram[0x09] = (ram[0x09] >> 1) | (carry ? 0x80 : 0x00);
    carry = new_carry;
    
    // ror $08
    new_carry = ram[0x08] & 1;
    ram[0x08] = (ram[0x08] >> 1) | (carry ? 0x80 : 0x00);
    carry = new_carry;
    
    // ror $07
    new_carry = ram[0x07] & 1;
    ram[0x07] = (ram[0x07] >> 1) | (carry ? 0x80 : 0x00);
    carry = new_carry;
    
    snes->cpu->c = carry;
    snes->cpu->a = ram[0x08];  // lda $08 — sets A low byte, preserves B
    snes->cpu->z = (ram[0x08] == 0);
    snes->cpu->n = (ram[0x08] & 0x80) != 0;
    
    return ram[0x08];
}