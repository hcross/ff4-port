// This routine clamps the accumulator to 255 if the carry flag is set.
// If Carry = 0, the current value of A is preserved.
// If Carry = 1, A is set to 0xFF.
static uint16_t Max255_c(Snes *snes) {
    Cpu *cpu = snes->cpu;

    // Entry mode: A 8-bit (mf=1), DB=$9E (Bank of the routine)
    // The routine relies on the Carry flag set by the preceding instruction
    // in the caller's flow (e.g., an ADC or CMP).
    if (cpu->c) {                // bcc @9e2b -> branch if carry clear
        cpu->a = 0xFF;           // lda #$ff
        // In 8-bit mode, loading #$ff sets Z=0, N=1
        cpu->z = false;
        cpu->n = true;
    }

    return cpu->a;
}

// PITFALLS: 2 (Routine starts with BCC; Carry flag must be set by caller)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9E
//   entry_flags: z=auto, n=auto, c=expr
// CUSTOM_SPIKE: yes (Output is in register A, not RAM)

REVERSED_FUNCTION: battle::Max255 ($9E:27)