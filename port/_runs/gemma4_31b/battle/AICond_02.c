// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0 (battle convention)
// Purpose: Check if the value at an indexed location in the AI table matches
//          the current target/condition and increment a counter if so.
//
// Logic:
//   index = ram[0x289E]
//   value = ram[0x35F3 + index]
//   if (value == ram[0x289F]) { ram[0xDE]++ }
static void AICond_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t index = ram[0x289E]; // lda $289e / tax
    uint8_t val = ram[0x35F3 + index]; // lda $35f3,x
    
    if (val == ram[0x289F]) {    // cmp $289f / bne @bdb7
        ram[0xDE]++;             // inc $de
    }
}

// PITFALLS: 8 (Inherited mf=true for battle code. A is 8-bit, allowing 
//           indexing with X and comparison of single bytes).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x289E=1, 0x289F=1, 0x35F3=1 (indexed by 0x289E)
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_02 ($BD:A9)