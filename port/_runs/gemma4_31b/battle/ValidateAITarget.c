// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Validates if an AI target is valid based on specific flags in the target's data.
// If the target is invalid (any flag set in the first two checks, or sign bit set in the third),
// ram[$35EA] is incremented (marked as invalid).
static void ValidateAITarget_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = snes->cpu->x;

    ram[0x35EA] = 0;

    // lda $2003,x / and #$c0
    // Check if bits 6 or 7 are set
    if ((ram[0x2003 + x] & 0xC0) != 0) {
        ram[0x35EA]++; // bne @c0f0
        return;
    }

    // lda $2005,x / and #$82
    // Check if bits 1 or 7 are set
    if ((ram[0x2005 + x] & 0x82) != 0) {
        ram[0x35EA]++; // bne @c0f0
        return;
    }

    // lda $2006,x / bpl @c0f3
    // bpl (Branch if Plus) occurs if bit 7 is CLEAR.
    // If bit 7 is SET (negative), it falls through to inc $35ea.
    if ((ram[0x2006 + x] & 0x80) != 0) {
        ram[0x35EA]++; // Fallthrough to @c0f0
    }
}

// PITFALLS: 1 (DB=$7E used for RAM access), 3 (BPL logic: branch if bit 7 is 0, 
// so we increment if bit 7 is 1)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16-bit, y=none
//   inputs_ram:  0x2003+x=1, 0x2005+x=1, 0x2006+x=1
//   output_ram:  0x35EA=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::ValidateAITarget ($C0:DA)