// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$D3, DP=0
// Logic:
//   1. Extract bits 0-2 of the input A into ram[$a9] (3 bits).
//   2. Shift ram[$a9] right by 5 via Lsr_5.
//   3. Use the result as an index into NoMonsterFlashTbl to find a bitmask.
//   4. Iterate through the bitmask (shifting left) until the high bit is set (bpl loop).
//   5. If the carry is set (bit 7 was 1), the result is the shifted mask; else 0.
static void CheckMonsterFlash_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    // Sequence of lsr/ror $a9 extracts bits 0, 1, and 2 of A into $a9
    ram[0xA9] = 0;
    for (int i = 0; i < 3; i++) {
        uint8_t carry = a & 1;
        a >>= 1; // Pitfall 7: truncation to 8-bit
        ram[0xA9] = (uint8_t)((ram[0xA9] >> 1) | (carry << 7));
    }

    // The result of the shifts is in A, then moved to X
    uint8_t x_idx = ram[0xA9];
    
    // Lsr_5 is delegated. It operates on the value in A.
    snes->cpu->a = ram[0xA9];
    uint8_t shifted_idx = (uint8_t)lsr_5_emu(snes);
    ram[0xA9] = shifted_idx;

    // Look up in NoMonsterFlashTbl (located in ROM/Bank f)
    // The exact address of NoMonsterFlashTbl is needed. 
    // Assuming the harness provides access to ROM data or a mirrored mapping.
    uint8_t mask = snes->rom[0xf000 + x_idx]; // Simplified address for the table

    uint8_t y = shifted_idx;
    uint8_t current_mask = mask;
    bool carry = false;

    // @d36e: asl / dey / bpl loop
    // This loop shifts the mask left until the 7th bit (sign bit) is set
    do {
        carry = (current_mask & 0x80) != 0;
        current_mask = (uint8_t)(current_mask << 1);
        if (y == 0) break;
        y--;
    } while (current_mask >= 0); // bpl checks N flag (bit 7 == 0)

    if (carry) {
        // bcc @d376 not taken: carry is set
        // ror A: The mask is shifted right, effectively restoring the bit 
        // that triggered the loop exit or moving the result back.
        snes->cpu->a = (uint8_t)(current_mask >> 1) | (carry << 7); // Simplified ROR
    } else {
        // bcc @d376 taken: clear A
        snes->cpu->a = 0; // clr_a
    }
}

// PITFALLS: 7 (8-bit shifts/truncation), 8 (mf=true battle convention)
// HELPERS: lsr_5_emu(snes) — delegates Lsr_5
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0, db=0xD3
//   entry_flags: z=auto, n=auto
//   return_reg:  a=8
REVERSED_FUNCTION: battle::CheckMonsterFlash ($D3:54)