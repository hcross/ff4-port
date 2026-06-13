#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C3 (or $7E per module), DP=0
// Logic: 
//   1. Loads a byte from $0E.
//   2. Shifts it right 3 times, propagating the bits into $3D (which was cleared).
//   3. This effectively performs: ram[0x3D] = ram[0x0E] >> 3;
//   4. Adds the value at $0C to this result and stores it back in $3D.
static void GetNPCMapPtr_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // a = ram[0x0E]
    uint8_t val = ram[0x0E];
    ram[0x3E] = val;
    ram[0x3D] = 0;

    // Sequence of 3x (lsr $3E / ror $3D)
    // In 8-bit mode, lsr $3E shifts ram[0x3E] right, MSB of $3D becomes the Carry flag.
    // ror $3D rotates Carry into MSB of $3D.
    // Since $3D started at 0 and we are shifting a value from $3E into it,
    // and the original asm is manipulating $3D (High) and $3E (Low) as a 16-bit pair:
    // [ $3D ][ $3E ] >> 3
    
    uint16_t pair = (uint16_t)(ram[0x3D] | (ram[0x3E] << 0)); // Simplified logic
    // Actually, the asm does:
    // lsr $3e (bit 0 gone, bit 7 -> C) / ror $3d (C -> bit 7, bit 0 -> C)
    // This is exactly a 16-bit right shift of the word formed by $3D:3E.
    
    uint16_t word = (uint16_t)(ram[0x3D] << 8 | ram[0x3E]);
    word >>= 3;
    
    ram[0x3E] = (uint8_t)(word & 0xFF);
    ram[0x3D] = (uint8_t)((word >> 8) & 0xFF);

    // lda $3d / clc / adc $0c / sta $3d
    uint8_t result = ram[0x3D];
    result = (uint8_t)(result + ram[0x0C]); // Pitfall 7: truncate to 8-bit
    ram[0x3D] = result;
}

// PITFALLS: 7 (Arithmetic truncation to 8-bit), 8 (mf=true assumed for field/battle modules)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0E=1, 0x0C=1
//   output_ram:  0x3D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC3
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetNPCMapPtr ($C3:0055)