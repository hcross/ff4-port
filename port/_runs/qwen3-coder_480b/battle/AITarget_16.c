// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: ram[$361C] = bit count, ram[$A6] = base offset
// Logic: Rotate carry bit into ($2053 + X) for 'bit count' iterations
static void AITarget_16_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t bit_count = ram[0x361C];         // lda $361c
    uint16_t y = bit_count;                  // tay
    uint16_t x = read16(ram, 0xA6);          // ldx $a6
    snes->cpu->c = true;                     // sec

    // Loop: ror ($2053,x) / dey / bpl loop
    for (; (int16_t)y >= 0; y--) {           // bpl = branch if y >= 0 (signed)
        uint8_t val = ram[0x2053 + x];
        uint8_t new_val = (uint8_t)((val >> 1) | (snes->cpu->c ? 0x80 : 0));
        snes->cpu->c = (val & 1) != 0;       // carry = LSB before shift
        ram[0x2053 + x] = new_val;           // ror $2053,x
        // dey is loop decrement
    }
}

// PITFALLS: 1 (DB=$7E), 6 (A 8-bit assumed), 7 (no hidden B issue — X is full 16-bit)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x361C=1, 0xA6=2
//   output_ram:  0x2053=1  ; actually 0x2053 + X, but spike must concretize
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: c=true, z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_16 ($B9:000A)