// Entry mode: A 8-bit (inherited as default for battle), X 16-bit (longi standard)
// Entry: A = 0 (lda #0 before jump)
// This function unconditionally jumps to TargetMonsterTypeAll with A=0
static void AITarget_19_c(Snes *snes) {
    // lda #0 sets A to zero
    snes->cpu->a = 0;
    // jmp TargetMonsterTypeAll → direct jump, no return
    TargetMonsterTypeAll_emu(snes);
}

// PITFALLS: 1 (DB must be $7E for TargetMonsterTypeAll if it accesses WRAM)
// HELPERS: TargetMonsterTypeAll_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_19 ($B9:38)