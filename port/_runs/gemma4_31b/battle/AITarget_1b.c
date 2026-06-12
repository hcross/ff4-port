// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine is a simple wrapper that sets the target type to 2
// (all monsters) and jumps to the target selection logic.
static void AITarget_1b_c(Snes *snes) {
    // lda #2
    snes->cpu->a = 2;
    
    // jmp TargetMonsterTypeAll
    // Since TargetMonsterTypeAll is a jump target, we delegate the 
    // logic starting from that address.
    target_monster_type_all_emu(snes);
}

// PITFALLS: None. Simple register load and jump.
// HELPERS: target_monster_type_all_emu(snes) — delegates TargetMonsterTypeAll @ $B9:3D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AITarget_1b ($B9:69)