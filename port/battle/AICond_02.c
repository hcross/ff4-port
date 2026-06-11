// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Reads from: ram[$289E] (index), ram[$289F] (value to compare)
// Compares ram[$35F3 + index] with ram[$289F]. If equal, increments ram[$DE].
static void AICond_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t index = ram[0x289E];              // lda $289e
    uint8_t value = ram[0x35F3 + index];      // tax / lda $35f3,x
    uint8_t target = ram[0x289F];             // cmp $289f
    if (value == target) {                    // bne @bdb7 → taken if !=, so body runs if ==
        ram[0xDE]++;                          // inc $de
    }
}

// PITFALLS: 1 (DB=$7E required for correct absolute addressing)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x289E=1, 0x289F=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
REVERSED_FUNCTION: battle::AICond_02 ($BD:A9)