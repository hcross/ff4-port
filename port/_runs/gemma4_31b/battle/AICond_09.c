// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: If ram[0x38D3] is non-zero, increment the counter at ram[0xDE].
static void AICond_09_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    if (ram[0x38D3] != 0) { // lda $38d3 / beq @beeb (inverted)
        ram[0xDE]++;        // inc $de
    }
}

// PITFALLS: None (simple 8-bit logic, no complex flags or mode transitions)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x38D3=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_09 ($BE:E4)