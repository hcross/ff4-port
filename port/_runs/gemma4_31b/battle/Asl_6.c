// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Performs 6 consecutive arithmetic shift lefts on the accumulator.
// This effectively multiplies the 8-bit value by 64, truncated to 8 bits.
static void Asl_6_c(Snes *snes) {
    Cpu *c = snes->cpu;
    uint8_t a = (uint8_t)c->a;

    for (int i = 0; i < 6; i++) {
        c->c = (a & 0x80) != 0;     // Carry is the bit shifted out
        a = (uint8_t)(a << 1);      // Pitfall 7: Truncate to 8-bit
    }

    c->a = a;
    c->z = (a == 0);
    c->n = (a & 0x80) != 0;
}

// PITFALLS: 7 (8-bit ASL truncation: (uint8_t) cast ensures bit 8 is 
// dropped to match 65816 8-bit shift behavior)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Asl_6 ($84:7B)