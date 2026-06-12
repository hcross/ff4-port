// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B9 (module bank), DP=0
// Logic: This routine performs a series of rotations (ROR) on a memory 
// region starting at $2053, indexed by ram[$a6]. The number of 
// rotations is determined by ram[$361C].
// The carry flag is set (SEC) before the loop starts.
static void AITarget_16_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    uint8_t count = ram[0x361C]; // lda $361c / tay
    uint16_t index = read16(ram, 0xA6); // ldx $a6
    
    bool carry = true; // sec
    
    // The loop runs while Y >= 0 (bpl @b911). 
    // Since Y is initialized to 'count' (unsigned 8-bit), 
    // the loop executes (count + 1) times.
    int8_t y = (int8_t)count;
    while (y >= 0) {
        uint8_t val = ram[0x2053 + index]; // ror $2053,x
        
        // 65816 ROR: bit 0 is shifted out to C, C is shifted into bit 7
        uint8_t next_val = (uint8_t)((val >> 1) | (carry << 7));
        carry = (val & 1) != 0;
        
        ram[0x2053 + index] = next_val;
        
        y--; // dey
    }
}

// PITFALLS: 7 (8-bit rotation truncation), 8 (Assumed mf=true for battle/AI logic)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x361C=1, 0x00A6=2
//   output_ram:  0x2053=1 (modified based on index $a6)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB9
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Output depends on variable index $a6 and loop count)

REVERSED_FUNCTION: battle::AITarget_16 ($B9:0A)