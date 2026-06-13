#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C8 (implied by code bank), DP=0
// This routine extracts two 2-bit values from a single byte at $1703,
// splitting them into $4A and $4B.
// 
// Logic:
// 1. Load byte from $1703.
// 2. Store 0 in $4A.
// 3. Shift A right (lsr), rotate into $4A (ror). Repeat twice.
//    This moves bits 0-1 of $1703 into $4A.
// 4. Shift A right (lsr), rotate into $4A (ror). Repeat twice.
//    This moves bits 2-3 of $1703 into $4A.
//    Wait, actually:
//    Cycle 1: lsr A (bit 0 -> C), ror $4A (C -> bit 0, $4A shifted).
//    The sequence is a manual bit-field extraction.
static void GetTopCharPtr_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    uint8_t a = ram[0x1703];
    ram[0x4A] = 0;

    // First 2-bit extraction
    // ASM: lsr / ror $4a / lsr / ror $4a
    for (int i = 0; i < 2; i++) {
        bool carry = (a & 1); // lsr A
        a = (uint8_t)(a >> 1); // Pitfall 7: truncate to 8-bit
        
        bool old_carry = (ram[0x4A] & 1); // ror $4a (bit 0 -> C)
        ram[0x4A] = (uint8_t)((ram[0x4A] >> 1) | (carry << 7)); // C -> bit 7
        // Note: 65816 ROR on memory treats it as a shift. 
        // The sequence effectively shifts bits from A into $4A.
    }
    
    // Correction: Let's trace the exact bit movement of lsr A / ror $4A
    // $1703 = [7 6 5 4 3 2 1 0]
    // 1. lsr A -> C=0, A=[0 7 6 5 4 3 2 1]
    // 2. ror $4A -> C=0, $4A=[0 0 0 0 0 0 0 0] (stz $4a happened)
    // This specific pattern is used to shift bits from A into the low bits of $4A.
    
    // Re-evaluating the ASM logic:
    // lsr / ror $4a -> A bit 0 moves to Carry, Carry moves to $4A bit 0 (since $4A was 0)
    // After 4 iterations: bits 0-3 of $1703 are shifted into $4A.
    
    // Let's implement the precise bit-shifting for parity:
    uint8_t val = ram[0x1703];
    uint8_t target = 0; // $4A
    
    // Sequence: lsr, ror $4a, lsr, ror $4a
    for (int i = 0; i < 4; i++) {
        bool c = (val & 1);
        val >>= 1;
        bool r_c = (target & 1);
        target = (target >> 1) | (c << 7); // This is the literal ROR behavior
        // But wait, the target is $4A. Let's trace the result:
        // Since $4A is 0, it's essentially (val & 0x0F) being shifted.
    }
    
    // Actually, the most idiomatic and parity-safe way to represent 65816 
    // bit-shifting of this nature is to simulate the carry:
    uint8_t a_reg = ram[0x1703];
    uint8_t ram_4a = 0;
    bool carry = false;

    // 1st lsr / ror $4a
    carry = (a_reg & 1); a_reg >>= 1;
    bool low_bit = (ram_4a & 1); ram_4a = (ram_4a >> 1) | (carry << 7); carry = low_bit;
    // 2nd lsr / ror $4a
    carry = (a_reg & 1); a_reg >>= 1;
    low_bit = (ram_4a & 1); ram_4a = (ram_4a >> 1) | (carry << 7); carry = low_bit;
    // 3rd lsr / ror $4a
    carry = (a_reg & 1); a_reg >>= 1;
    low_bit = (ram_4a & 1); ram_4a = (ram_4a >> 1) | (carry << 7); carry = low_bit;
    // 4th lsr / ror $4a
    carry = (a_reg & 1); a_reg >>= 1;
    low_bit = (ram_4a & 1); ram_4a = (ram_4a >> 1) | (carry << 7); carry = low_bit;

    ram[0x4B] = a_reg;
    ram[0x4A] = ram_4a;
    snes->cpu->x = ram_4a; 
}

// PITFALLS: 7 (8-bit shift truncation)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1703=1
//   output_ram:  0x4A=1, 0x4B=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC8
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetTopCharPtr ($C8:7B)