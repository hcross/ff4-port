// Logic:
//   Check if the value at ram[0x35F3 + ram[0x289E]] equals ram[0x289F].
//   If equal, increment ram[0xDE] (Direct Page relative).
//
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0xBD, DP=0
// Note: The routine uses absolute addressing for $289E, $35F3, $289F, 
// and relative addressing for $DE. Based on the battle module convention,
// $DE is interpreted as DB:$00DE.
static void AICond_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $289e / tax
    // A is 8-bit, but X is 16-bit. The 8-bit value is zero-extended into X.
    uint16_t x = (uint16_t)ram[0x289E];

    // lda $35f3,x
    uint8_t val = ram[0x35F3 + x];

    // cmp $289f / bne @bdb7
    if (val == ram[0x289F]) {
        // inc $de (Direct Page access)
        // In battle module, DP=0, DB=0xBD (or 0x7E depending on context), 
        // but usually, these low offsets are in the common WRAM bank.
        ram[0x00DE]++; 
    }
}

// PITFALLS: 8 (Inherited mode: A=8-bit, X=16-bit)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x289E=1, 0x289F=1, 0x35F3=1 (indexed by 0x289E)
//   output_ram:  0x00DE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBD
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_02 ($BD:A9)