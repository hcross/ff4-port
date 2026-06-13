#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This function computes the sum of two 16-bit values stored in
// $6C/$6D and $7C/$7D, storing the result in $4B/$4C and $4D/$4E.
static void GetEarthSpritePos_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // longa: Switch to 16-bit A mode
    // lda $6c: Load 16-bit value from $6C
    uint16_t a = read16(ram, 0x6C);
    // clc: Clear carry
    // adc $7c: Add 16-bit value from $7C to A
    a += read16(ram, 0x7C);
    // sta $4b: Store result in $4B
    write16(ram, 0x4B, a);

    // lda $6e: Load 16-bit value from $6E
    a = read16(ram, 0x6E);
    // clc: Clear carry
    // adc $7e: Add 16-bit value from $7E to A
    a += read16(ram, 0x7E);
    // sta $4d: Store result in $4D
    write16(ram, 0x4D, a);

    // shorta0: Set A to 8-bit mode (and clear high byte)
}

// PITFALLS: 6 (16-bit A mode operations), 1 (DB=$7E for absolute addressing)
// HELPERS: read16, write16 for 16-bit memory access
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x6C=2, 0x7C=2, 0x6E=2, 0x7E=2
//   output_ram:  0x4B=2, 0x4D=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::GetEarthSpritePos ($E1:39)