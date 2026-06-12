// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine checks if the values at $289E-289F match the values at $1800-1801.
// If both match, it increments a counter at $DE.
static void AICond_05_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $289e / cmp $1801 / bne @be30
    if (ram[0x289E] != ram[0x1801]) {
        return;
    }

    // lda $289f / cmp $1800 / bne @be30
    if (ram[0x289F] != ram[0x1800]) {
        return;
    }

    // inc $de
    ram[0xDE]++;
}

// PITFALLS: None. (Simple 8-bit comparisons, no complex branching or register pollution)
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x289E=1, 0x289F=1, 0x1800=1, 0x1801=1
//   output_ram:  0x00DE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_05 ($BE:1E)