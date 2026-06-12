// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$85, DP=0
// This routine performs a population count (popcount) of the 8-bit accumulator.
// It shifts the accumulator left 8 times, incrementing X whenever the Carry flag is set.
// Entry: cpu->a = value to check
// Exit: cpu->x = number of set bits (0-8)
static void CountBits_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t val = (uint8_t)cpu->a;
    uint16_t count = 0; // ldx #0

    for (uint8_t y = 8; y > 0; y--) { // ldy #8 / dey / bne loop
        // asl A: The bit shifted out of bit 7 goes into the carry flag
        bool carry = (val & 0x80) != 0;
        val = (uint8_t)(val << 1); // Pitfall 7: truncate to 8-bit to match 65816 behavior

        if (carry) {               // bcc @8516: branch if carry clear, so enter if carry set
            count++;                // inx
        }
    }
    cpu->x = count;
}

// PITFALLS: 7 (asl A results truncated to uint8_t to prevent 16-bit promotion)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x85
//   entry_flags: z=auto, n=auto
//   output_reg:  x=16
REVERSED_FUNCTION: battle::CountBits ($85:0C)