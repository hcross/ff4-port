#include "snes/snes.h"

// Rotates a 24-bit value right by one bit through three consecutive bytes.
// Entry: A (16-bit) contains the value to be rotated, stored in $07-$09.
//        Assumes A is 16-bit (mf=0), DB=0x7E, DP=0.
//        No flags are consulted at entry.
static void RotateWord_c(Snes *snes, uint16_t value) {
    uint8_t *ram = snes->ram;
    // Store the 16-bit value into $07 and $09 (low and high bytes)
    ram[0x07] = value & 0xFF;
    ram[0x08] = 0;  // middle byte is zeroed
    ram[0x09] = (value >> 8) & 0xFF;

    // Perform the rotation: ROR $09 -> $08 -> $07
    uint8_t c = (ram[0x09] & 1);           // Capture carry from $09
    ram[0x09] = (ram[0x09] >> 1) | (ram[0x08] << 7);
    ram[0x08] = (ram[0x08] >> 1) | (ram[0x07] << 7);
    ram[0x07] = (ram[0x07] >> 1) | (c << 7);

    // Load result from $08 into A (low byte of result)
    snes->cpu->a = ram[0x08];
}

// PITFALLS: 6 (16-bit A assumed), 1 (DB must be $7E for correct addressing)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=16, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x07=1, 0x08=1, 0x09=1
//   entry_mode:  mf=false, xf=true, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::RotateWord ($8B:1D)