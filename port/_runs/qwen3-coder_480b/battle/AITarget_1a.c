// Entry mode: A 8-bit (inherited as default for battle module), X 16-bit
// Entry: none (no inputs in registers)
// This function unconditionally jumps to TargetMonsterTypeAll with A=1
static void AITarget_1a_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    cpu->a = 1;                        // lda #1
    // Jump to TargetMonsterTypeAll (delegate)
    target_monster_type_all_emu(snes);
}

// PITFALLS: 1 (DB must be $7E for battle module when delegating),
//           2 (Z/N flags not relevant as no branch-on-register at entry)
// HELPERS: target_monster_type_all_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_1a ($B9:64)