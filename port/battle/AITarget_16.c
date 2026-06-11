// Rotates right the byte at $2053,x 'y+1' times, with carry.
// Entry: A = $361C (loop counter, 8-bit), X = $A6 (base offset, 16-bit)
//        Y is not an input, it's loaded from A.
//        Carry is set before the loop (sec).
static void AITarget_16_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t y = ram[0x361C];     // lda $361c / tay
    uint16_t x = read16(ram, 0xA6); // ldx $a6 (X is 16-bit)
    // sec — carry is set before loop
    uint8_t addr = 0x2053 + (x & 0xFFFF); // 8-bit address wrap assumed
    uint8_t val = ram[addr];
    for (;;) {
        val = (val >> 1) | ((snes->cpu->c_flag ? 1 : 0) << 7); // ror
        snes->cpu->c_flag = (val & 0x80) != 0; // update carry from MSB
        ram[addr] = val;
        if (y == 0) break;
        y--; // dey / bpl
    }
}

// PITFALLS: 6 (A is 8-bit, X is 16-bit), 7 (8-bit ROR truncation),
//           4 (address wrap on 8-bit addr + X)
// HELPERS: read16 for 16-bit X load
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  0x361C=1, 0xA6=2
//   output_ram:  0x2053+x=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: c=1, z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_16 ($B9:0A)