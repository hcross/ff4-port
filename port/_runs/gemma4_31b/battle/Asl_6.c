// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine performs 6 consecutive shifts left (ASL) on the accumulator.
// In 8-bit mode, this is equivalent to (A << 6) truncated to 8 bits.
// The carry flag is updated by the final shift.
static uint16_t Asl_6_c(Snes *snes) {
    Cpu *c = snes->cpu;
    uint8_t a = (uint8_t)c->a;

    // Perform 6 shifts. Pitfall 7: Result must be truncated to 8-bit.
    for (int i = 0; i < 6; i++) {
        bool carry = (a & 0x80) != 0;
        a = (uint8_t)(a << 1); // Pitfall 7: cast to uint8_t to drop bit 8
        c->c = carry;
    }

    // Update Z and N flags based on the final 8-bit result
    c->z = (a == 0);
    c->n = (a & 0x80) != 0;
    
    c->a = a;
    return c->a;
}

// PITFALLS: 7 (8-bit ASL truncation: explicit cast to uint8_t used to 
// ensure bit 8 is dropped and not promoted to 16-bit int)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: no
REVERSED_FUNCTION: battle::Asl_6 ($84:7B)