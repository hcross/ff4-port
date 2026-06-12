// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B9, DP=0
// Logic: Sets A to 1 and jumps to TargetMonsterTypeAll.
// Since TargetMonsterTypeAll is a jump target (not a subroutine), 
// we treat the transition as a call to its emulated counterpart.
static void AITarget_1a_c(Snes *snes) {
    snes->cpu->a = 1;
    target_monster_type_all_emu(snes);
}

// PITFALLS: None.
// HELPERS: target_monster_type_all_emu(snes) — delegates TargetMonsterTypeAll @ $B9:3D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB9
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AITarget_1a ($B9:64)