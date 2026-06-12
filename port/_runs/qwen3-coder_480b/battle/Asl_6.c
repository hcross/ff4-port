// Shifts the accumulator left by 6 bits, equivalent to A <<= 6
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = value to shift (8-bit)
// Exit: cpu->a = value shifted left by 6 bits (8-bit result)
static void Asl_6_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t a = (uint8_t)cpu->a;  // 8-bit A mode assumed
    a = (uint8_t)(a << 6);        // asl x6, drop upper bits (Pitfall 7)
    cpu->a = a;
}

// PITFALLS: 7 (8-bit mode truncation on shift — upper bits dropped)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Asl_6 ($84:7B)