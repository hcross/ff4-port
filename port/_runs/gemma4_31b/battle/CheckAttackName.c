// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   1. Extracts bits 3-5 of the input attack ID into A (low) and $a9 (high).
//   2. Uses Lsr_5 to shift this 3-bit value right by 5 (effectively clearing it, 
//      but we call the helper to maintain parity).
//   3. Checks if the attack ID is present in the NoNameAttackTbl bitmask.
//   4. If the bit is set, returns 0; otherwise returns the original attack ID.
static uint16_t CheckAttackName_c(Snes *snes, uint8_t attack_id) {
    uint8_t *ram = snes->ram;
    
    // The routine preserves A on stack and works with it
    uint8_t original_a = attack_id;
    
    // Extract 3 bits from attack_id into a 16-bit value split across A and $a9
    // Sequence: lsr A / ror $a9 (repeated 3 times)
    uint8_t a = original_a;
    uint8_t a9 = 0;
    for (int i = 0; i < 3; i++) {
        uint8_t carry = (a & 1);
        a >>= 1; // Pitfall 7: (uint8_t) implicit
        a9 = (uint8_t)((a9 >> 1) | (carry << 7));
    }

    // tax / lda $a9
    uint16_t x = (uint16_t)a; // X is 16-bit
    uint8_t current_a = a9;

    // jsr Lsr_5
    // Lsr_5 is a delegated helper that shifts the accumulator
    uint16_t res = lsr_5_emu(snes); 
    current_a = (uint8_t)res;
    ram[0xA9] = current_a;

    // lda $a9 / tay / lda NoNameAttackTbl,x
    uint8_t y = current_a;
    // NoNameAttackTbl is a ROM table. The index x (bits 0-2 of attack_id) 
    // is used to fetch the mask byte.
    // Assuming NoNameAttackTbl address is resolved via external ROM mapping
    uint8_t mask = snes->rom[NO_NAME_ATTACK_TBL_ADDR + x];

    // Search loop: asl / dey / bpl @d349
    // This iterates through the bits of the mask byte
    while (y != 0 || (int8_t)y >= 0) {
        uint8_t carry = (mask & 0x80) != 0;
        mask <<= 1; // Pitfall 7
        y--;
        if (y < 0) {
            // bcc @d352: if carry is clear, jump to @d352 (return original)
            if (!carry) {
                return (uint16_t)original_a;
            } else {
                // Carry set: attack is in the "No Name" table
                return 0; // clr_a / rts
            }
        }
    }
    
    return (uint16_t)original_a;
}

// PITFALLS: 7 (Arithmetic/shift truncation in 8-bit mode for lsr/asl)
// HELPERS: lsr_5_emu(snes) — delegates Lsr_5
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (returns value in A instead of RAM)
REVERSED_FUNCTION: battle::CheckAttackName ($D3:2E)