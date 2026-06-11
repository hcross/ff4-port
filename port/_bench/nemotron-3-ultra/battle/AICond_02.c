// battle::AICond_02 — AI condition check: if ram[$35F3 + ram[$289E]] == ram[$289F], increment $DE
// Entry mode: A 8-bit (mf=1), X 8-bit (xf=1), DB=$7E, DP=0
// No register I/O; all operands in WRAM. First instruction is LDA (not a branch), so no entry flag contract.
static void AICond_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t idx = ram[0x289E];
    if (ram[0x35F3 + idx] == ram[0x289F]) {
        ram[0xDE]++;
    }
}

// PITFALLS: 1 (DB=$7E required for WRAM addresses)
// HELPERS: (none)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x289E=1, 0x35F3=1 (table base, indexed by 0x289E), 0x289F=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_02 ($BD:A9)