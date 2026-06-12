// Logic:
// Increments the value in register X by 0x0080 (128) and updates A.
// Note: The routine explicitly forces 16-bit mode for the addition, 
// then resets A to the value of the D-register (usually 0) 
// and switches A back to 8-bit mode.
static void NextObj_c(Snes *snes) {
    Cpu *cpu = snes->cpu;

    // longa: A becomes 16-bit
    cpu->mf = false;

    // txa / clc / adc #$0080 / tax
    // X is 16-bit (xf=false per battle convention), 
    // so we perform 16-bit addition and store back to X.
    cpu->x = (uint16_t)(cpu->x + 0x0080);

    // shorta0: clr_a / shorta
    // clr_a (tdc): A = Direct Page register
    cpu->a = cpu->dp; 
    // shorta: A becomes 8-bit
    cpu->mf = true;
}

// PITFALLS: 6 (Mode A 16-bit for addition), 8 (Battle convention xf=false)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   exit_regs:   x=x+0x80, a=dp
REVERSED_FUNCTION: battle::NextObj ($85:BC)