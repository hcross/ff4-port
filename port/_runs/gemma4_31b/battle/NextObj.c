// Logic:
// Increments the index in X by 0x80 (the size of one object entry in 
// the object table) and returns. The routine ensures 16-bit arithmetic 
// for the pointer shift and then resets A to the DP value (typically 0) 
// and reverts A to 8-bit mode.
static void NextObj_c(Snes *snes) {
    Cpu *cpu = snes->cpu;

    // longa: A 16-bit
    cpu->mf = false;

    // txa / clc / adc #$0080 / tax
    // X is 16-bit per battle module convention (xf=false)
    cpu->x = (uint16_t)(cpu->x + 0x0080);

    // shorta0: clr_a / shorta
    // clr_a is tdc (transfer direct page to A)
    cpu->a = cpu->dp;
    // shorta: A 8-bit
    cpu->mf = true;
}

// PITFALLS: 6 (Mode A 16-bit), 8 (X 16-bit heritage)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   exit_regs:   x=x+0x80, a=dp
REVERSED_FUNCTION: battle::NextObj ($85:BC)