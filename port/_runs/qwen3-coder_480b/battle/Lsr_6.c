// This function performs a logical shift right by 6 bits on the accumulator.
// Entry: A register (8-bit or 16-bit depending on mode)
// Exit: A register shifted right by 6 bits
static void Lsr_6_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    cpu->a >>= 6;                    // Perform 6 right shifts
    cpu->c = (cpu->a & 0x20) != 0;   // Set carry flag based on bit 5 (before shift)
    cpu->z = (cpu->a == 0);          // Set zero flag
    cpu->n = (cpu->a & 0x80) != 0;   // Set negative flag (for 8-bit)
}

// PITFALLS: 6 (mode A size matters - assuming 8-bit), 7 (truncation in 8-bit mode)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Lsr_6 ($84:82)