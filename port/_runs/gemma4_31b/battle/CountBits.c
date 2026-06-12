// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0)
// This routine counts the number of set bits (population count) in the 8-bit accumulator.
// Entry: cpu->a = value to check
// Exit: cpu->x = count of bits set (0-8)
static void CountBits_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t val = (uint8_t)cpu->a;
    uint16_t count = 0; // ldx #0

    for (uint8_t y = 8; y > 0; y--) { // ldy #8 / dey / bne loop
        uint8_t bit = (val & 0x80) >> 7; // asl A (bit 7 goes to Carry)
        val = (uint8_t)(val << 1);      // Pitfall 7: truncate to 8-bit
        
        if (bit != 0) {                  // bcc @8516 (branches if C=0, so we inc if C=1)
            count++;                    // inx
        }
    }
    cpu->x = count;
}

// PITFALLS: 7 (Truncation of asl result to 8-bit to avoid 16-bit promotion)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   output_reg:  x=16
REVERSED_FUNCTION: battle::CountBits ($85:0C)