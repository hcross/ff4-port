// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$D3, DP=0
// Logic:
//   1. Extract bits 0-2 of the input A into ram[$a9] via a series of LSR/ROR.
//   2. Shift the extracted value right by 5 using Lsr_5_emu.
//   3. Use the original extracted 3-bit value (in X) as an index into NoMonsterFlashTbl.
//   4. Iteratively shift the table value left (ASL) and decrement Y (the shifted index)
//      until the high bit is set (BPL loop).
//   5. If a bit was shifted into carry, return the value ROR'd; otherwise return 0.
static void CheckMonsterFlash_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    // stz $a9
    ram[0xA9] = 0;

    // Extract bits 0, 1, 2 of A into $a9 (effectively: ram[0xA9] = (a & 7) << 5)
    // Sequence: lsr A / ror $a9 (x3)
    for (int i = 0; i < 3; i++) {
        bool carry = (a & 1);
        a >>= 1; // Pitfall 7: 8-bit shift
        uint8_t val_a9 = ram[0xA9];
        bool carry_a9 = (val_a9 & 1);
        ram[0xA9] = (uint8_t)((val_a9 >> 1) | (carry << 7));
        // Note: ROR on memory in 65816 pushes the low bit of target into C, 
        // and C into the high bit of target.
    }

    uint8_t x_idx = ram[0xA9]; // tax
    snes->cpu->a = ram[0xA9];  // lda $a9
    
    Lsr_5_emu(snes);           // jsr Lsr_5
    
    uint8_t shifted_idx = (uint8_t)snes->cpu->a;
    ram[0xA9] = shifted_idx;   // sta $a9
    
    uint8_t y_val = shifted_idx; // lda $a9 / tay
    
    // Table lookup: f:NoMonsterFlashTbl (Bank $F)
    // The index used is x_idx (the bits extracted before Lsr_5)
    uint8_t mask = snes->rom[0xF0000 + x_idx]; // ROM Bank F mapping
    
    uint8_t current_mask = mask;
    bool carry = false;

    // @d36e: asl / dey / bpl loop
    // This loop shifts the mask left until the 7th bit is set or Y hits 0.
    do {
        carry = (current_mask & 0x80) != 0;
        current_mask = (uint8_t)(current_mask << 1); // Pitfall 7
        if (y_val == 0) break;
        y_val--;
    } while ((current_mask & 0x80) == 0); // bpl checks N flag (bit 7 == 0)

    if (carry) {
        // bcc @d376 not taken: carry is set
        // ror A: Rotate Carry into bit 7, shift bits right
        snes->cpu->a = (uint8_t)((current_mask >> 1) | (carry << 7));
    } else {
        // bcc @d376 taken
        snes->cpu->a = 0; // clr_a (tdc where DP=0)
    }
}

// PITFALLS: 7 (8-bit arithmetic truncation), 8 (mf=true battle convention)
// HELPERS: Lsr_5_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x00A9=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0xD3
//   entry_flags: z=auto, n=auto
//   return_reg:  a=8
REVERSED_FUNCTION: battle::CheckMonsterFlash ($D3:54)