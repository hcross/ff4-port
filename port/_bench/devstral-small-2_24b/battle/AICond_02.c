// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Check if a specific AI condition is met and increment $de if true.
//   Inputs: ram[$289E] = index, ram[$289F] = value to compare
//   Output: ram[$de] incremented if ram[$35F3 + index] == ram[$289F]
static void AICond_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t index = ram[0x289E];
    uint8_t value = ram[0x289F];
    uint8_t compared = ram[0x35F3 + index];
    if (compared == value) {  // cmp $289f / bne @bdb7 (inverted)
        ram[0xDE]++;
    }
}

// PITFALLS: 3 (CMP/BNE inversion: bne branches when A != mem, so we
// enter the body when A == mem)
// HELPERS: none
// CONTRACT:
//   inputs_ram:  0x289E=1, 0x289F=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none (no conditional branch at entry)
REVERSED_FUNCTION: battle::AICond_02 ($BD:A9)