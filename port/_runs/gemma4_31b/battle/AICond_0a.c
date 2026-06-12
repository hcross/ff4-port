// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$BE, DP=0
// Logic:
// 1. Calculate index: x = (ram[$d2] - 5) << 1
// 2. Load 16-bit damage value from ram[$34D4 + x]
// 3. If damage == 0, return.
// 4. If damage bits 6 or 7 are set (meaning MP damage or HP restoration), return.
// 5. Otherwise, increment ram[$de].
static void AICond_0a_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // sec / lda $d2 / sbc #$05
    // Pitfall 7: 8-bit arithmetic truncation
    uint8_t val = (uint8_t)(ram[0xD2] - 5); 
    
    // asl / tax
    // The result is shifted left to create a word-offset for the 16-bit damage values
    uint16_t x = (uint16_t)val << 1;

    // lda $34d4,x / ora $34d5,x
    // This checks if the 16-bit word at $34D4+x is non-zero
    uint16_t damage = read16(ram, 0x34D4 + x);
    if (damage == 0) { // beq @bf04
        return;
    }

    // lda $34d5,x / and #$c0
    // Check the high byte of the damage value for flags (bits 6 and 7)
    uint8_t high_byte = ram[0x34D5 + x];
    if ((high_byte & 0xC0) != 0) { // bne @bf04
        return;
    }

    // inc $de
    ram[0xDE]++;
}

// PITFALLS: 7 (8-bit arithmetic truncation on SBC and ASL)
// HELPERS: read16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00D2=1, 0x34D4=2 (indexed)
//   output_ram:  0x00DE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBE
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_0a ($BE:EC)