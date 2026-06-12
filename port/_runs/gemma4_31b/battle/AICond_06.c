// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: 
//   Calculates an index (ram[0xD2] - 5).
//   Fetches a value from a table starting at $29B5 using that index.
//   Uses that value as a second index to fetch a value from a table starting at $29CA.
//   If the fetched value matches the constant at $29CD, increments ram[0xDE].
static void AICond_06_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // sec / lda $d2 / sbc #$05
    // Subtracting 5 from ram[0xD2]. Result is treated as 8-bit.
    uint8_t index = (uint8_t)(ram[0xD2] - 5);

    // lda $29b5,x / tax
    // Fetch from table at $29B5 using the computed index.
    uint8_t table_val = ram[0x29B5 + index];

    // lda $29ca,x / cmp $29cd
    // Use the previously fetched value as an index for the table at $29CA.
    uint8_t target_val = ram[0x29CA + table_val];
    uint8_t comparison_val = ram[0x29CD];

    if (target_val == comparison_val) { // bne @be45 (branch if NOT equal)
        ram[0xDE]++;                   // inc $de
    }
}

// PITFALLS: 7 (Subtraction and index wrap-around treated as uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00D2=1, 0x29B5=1, 0x29CA=1, 0x29CD=1
//   output_ram:  0x00DE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_06 ($BE:31)