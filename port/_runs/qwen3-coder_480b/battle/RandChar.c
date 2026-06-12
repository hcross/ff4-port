// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: none (no inputs from registers or RAM)
// Output: none (output is returned in A register, caller reads it)
static void RandChar_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    cpu->x = 0;                          // ldx #0
    cpu->a = 4;                          // lda #4
    RandXA_emu(snes);                    // jsr RandXA
    // Result is in cpu->a (8-bit), caller reads it
}

// PITFALLS: 1 (DB=$7E required for RandXA), 8 (mode A/X inherited from caller)
// HELPERS: RandXA_emu(snes) — delegates RandXA @ $03:8379
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::RandChar ($03:8582)