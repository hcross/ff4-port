#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: 
//   Calculates an index based on value at $D2 (current monster index?).
//   Checks the damage taken by that monster at offset $34D4 + (val-5)*2.
//   If the damage is non-zero and doesn't have the high bits set (MP/Restored), 
//   it increments a flag at $DE.
static void AICond_0a_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // sec / lda $d2 / sbc #$05
    // Note: sbc with carry set is effectively (A - 5)
    int8_t val = (int8_t)ram[0xD2] - 5;

    // asl (A 8-bit) -> multiply by 2 to get word-offset
    // Pitfall 7: truncate to 8-bit to match ASM behavior
    uint8_t x_idx = (uint8_t)(val << 1);

    // lda $34d4,x / ora $34d5,x
    // This checks if any bits are set in the 16-bit word at 0x34D4 + x_idx
    uint16_t damage = read16(ram, 0x34D4 + x_idx);
    if (damage == 0) { // beq @bf04
        return;
    }

    // lda $34d5,x / and #$c0
    // Check high byte bits 6 and 7
    uint8_t high_byte = ram[0x34D5 + x_idx];
    if ((high_byte & 0xC0) != 0) { // bne @bf04
        return;
    }

    // inc $de
    ram[0xDE]++;
}

// PITFALLS: 7 (asl A in 8-bit mode truncated to uint8_t)
// HELPERS: read16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00D2=1, 0x34D4=2 (indexed by D2)
//   output_ram:  0x00DE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::AICond_0a ($BE:EC)