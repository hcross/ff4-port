// Compares ram[$35F3 + ram[$289E]] with ram[$289F]; if equal, increments ram[$DE]
// Entry mode: A 8-bit (mf=true), X 16-bit (xf=false), DB=$7E, DP=0
static void AICond_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    uint8_t a = ram[0x289E];                  // lda $289e
    uint16_t x = (uint16_t)a;                 // tax (zero-extends to 16-bit X)
    a = ram[0x35F3 + x];                      // lda $35f3,x
    
    if (a != ram[0x289F]) {                   // cmp $289f / bne @bdb7
        return;
    }
    ram[0xDE]++;                              // inc $de
}

// PITFALLS: 3 (CMP/BNE inversion), 8 (inherited mode: battle default mf=true, xf=false)
// HELPERS: (none)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x289E=1, 0x289F=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_02 ($03:BDA9)