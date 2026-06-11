// Purpose: Evaluate a common AI condition: if the character's
// battle status byte ($289E) matches a target value ($289F),
// increment the "condition met" counter at $DE.

// All memory accesses are in WRAM (DB=$7E). A is 8-bit, X is 16-bit
// (inherited from battle module conventions). The high byte of X at
// entry is preserved through the `tax` instruction (Pitfall 9) and
// affects the indexed load at $35F3+X.
static void AICond_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = snes->cpu->x;                    // full 16-bit X at entry

    uint8_t a = ram[0x289E];                      // lda $289e
    uint16_t new_x = (x & 0xFF00) | a;            // tax (high byte preserved)
    a = ram[0x35F3 + new_x];                      // lda $35f3,x
    if (a == ram[0x289F]) {                       // cmp $289f / bne (inverted)
        ram[0xDE]++;                              // inc $de
    }
}

// PITFALLS: 9 (tax preserves X high byte)
// HELPERS: (none)
// CONTRACT:
//   inputs_reg:  x=16
//   inputs_ram:  0x289E=1, 0x289F=1
//   output_ram: 0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
// CUSTOM_SPIKE: yes (indexed load address $35F3+X depends on input X)
REVERSED_FUNCTION: battle::AICond_02 ($BD:A9)