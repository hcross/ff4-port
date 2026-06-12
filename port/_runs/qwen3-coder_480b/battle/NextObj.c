// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->x = object pointer (16-bit)
// Exit: cpu->x = object pointer + 0x80 (16-bit)
static void NextObj_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    cpu->mf = false;              // longa
    cpu->x = (uint16_t)(cpu->x + 0x80);  // txa / clc / adc #$0080 / tax
    cpu->mf = true;               // shorta0
}

// PITFALLS: 6 (mode A is 16-bit due to `longa`), 7 (arithmetic truncation
// not needed since we're in 16-bit mode)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::NextObj ($85:BC)