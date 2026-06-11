// AICond_02: compare a byte from an indexed table ($35F3+X) against $289F;
// if equal, increment $DE. Used in AI condition evaluation.
//
// Entry mode: A 8-bit (mf=true), X/Y 16-bit (xf=false), DB=$7E, DP=0.
// Assumes B (hidden upper byte of C) = 0 on entry so that `tax` after
// `lda $289e` zero-extends the index.  (If parity fails, suspect Pitfall 9.)
static void AICond_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t index = ram[0x289E];          // lda $289e / tax  (X = index, B=0)
    uint8_t table_val = ram[0x35F3 + index]; // lda $35f3,x
    if (table_val == ram[0x289F]) {       // cmp $289f / bne (inverted)
        ram[0xDE]++;                      // inc $de
    }
}

// PITFALLS: 3 (CMP/BCS inversion — bne branches when not equal, so we
// enter the body when equal).  Pitfall 9 (B propagation) is assumed
// absent; if parity fails, the routine may need delegation.
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x289E=1, 0x289F=1, 0x35F3+index=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_02 ($BD:A9)