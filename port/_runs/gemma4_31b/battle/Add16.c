// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine performs a 32-bit addition of two 16-bit values
// stored in WRAM.
//   Input A:  $3956 (16-bit LE), $3958 (16-bit LE)
//   Output:   $395A (16-bit LE), $395C (16-bit LE)
static void Add16_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // longa: Enter 16-bit mode
    // clc: Clear carry
    uint16_t a = read16(ram, 0x3956);
    uint16_t b = read16(ram, 0x3958);
    
    uint32_t sum = (uint32_t)a + (uint32_t)b;
    
    // sta $395a: store low 16 bits of result
    write16(ram, 0x395A, (uint16_t)(sum & 0xFFFF));
    
    // lda #0 / adc #0 / sta $395c: capture carry overflow into high word
    uint16_t carry = (uint16_t)(sum >> 16);
    write16(ram, 0x395C, carry);

    // shorta0: A = 0, then return to 8-bit mode
    snes->cpu->a = 0;
    snes->cpu->mf = true;
}

// PITFALLS: 6 (Explicit longa/shorta ensures 16-bit arithmetic regardless of caller)
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3956=2, 0x3958=2
//   output_ram:  0x395A=2, 0x395C=2
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Add16 ($84:E3)