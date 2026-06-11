// Sets A to 2 and jumps to TargetMonsterTypeAll.
// Entry mode: A 8-bit (inherited), X 16-bit (battle convention)
// No flags or RAM inputs; output determined by TargetMonsterTypeAll
static void AITarget_1b_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    cpu->a = 2;                            // lda #2
    cpu->mf = true;                        // ensure A is 8-bit for emulation
    run_emulated_func(snes, 0xB93Du);      // jmp TargetMonsterTypeAll
}

// PITFALLS: 1 (DB must be $7E for battle), 2 (Z/N not relevant here)
// HELPERS: run_emulated_func (for TargetMonsterTypeAll)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=auto, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_1b ($B9:69)