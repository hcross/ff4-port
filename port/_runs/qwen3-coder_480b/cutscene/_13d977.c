#include "snes/snes.h"

// This function performs a 64-bit left rotation through carry on a multi-byte value
// stored in memory starting at $2703,x for 8 bytes. Each ROL shifts the carry
// into the low bit and shifts the high bit into carry.
static void _13d977_c(Snes *snes, uint16_t x) {
    uint8_t *ram = snes->ram;
    uint16_t base_addr = 0x2703 + x;

    // Perform 64-bit rotation through carry (8 consecutive ROLs)
    // Each ROL shifts the carry flag into bit 0 and bit 7 into carry
    uint8_t carry = snes->cpu->c;  // Get initial carry state

    for (int i = 0; i < 8; i++) {
        uint8_t val = ram[base_addr + i];
        uint8_t new_carry = (val >> 7) & 1;  // Save bit 7 for next carry
        val = (uint8_t)((val << 1) | carry); // Shift left and insert carry
        ram[base_addr + i] = val;
        carry = new_carry;  // Update carry for next iteration
    }

    // Update final carry flag
    snes->cpu->c = carry;
}

// PITFALLS: 7 (arithmetic/shift truncation in 8-bit mode - though this is a shift operation,
//            we must ensure we're working with 8-bit values only)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x2703+X=1, 0x2704+X=1, 0x2705+X=1, 0x2706+X=1, 0x2707+X=1, 0x2708+X=1, 0x2709+X=1, 0x270A+X=1
//   output_ram:  0x2703+X=1, 0x2704+X=1, 0x2705+X=1, 0x2706+X=1, 0x2707+X=1, 0x2708+X=1, 0x2709+X=1, 0x270A+X=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: c=auto
// REVERSED_FUNCTION: cutscene::_13d977 ($D9:77)