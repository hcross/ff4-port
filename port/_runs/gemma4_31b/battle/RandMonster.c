// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$85, DP=0
// Logic:
//   Sets X = 0, A = 7, and calls RandXA to generate a random value 
//   between X and A. The result is left in the accumulator.
static void RandMonster_c(Snes *snes) {
    Cpu *cpu = snes->cpu;

    cpu->x = 0;       // ldx #0
    cpu->a = 7;       // lda #7

    RandXA_emu(snes); // jsr RandXA
}

// PITFALLS: None applicable.
// HELPERS: RandXA_emu(snes) — delegates RandXA @ $8379
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x85
//   entry_flags: z=auto, n=auto
//   returns:     none (Result is in cpu->a)

REVERSED_FUNCTION: battle::RandMonster ($85:79)