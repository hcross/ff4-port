// Entry mode: A 8-bit (mf=true), X 16-bit (xf=false), DB=$B9, DP=0
// This routine is a wrapper that sets the target type to 0 (All)
// and jumps to the common target selection logic.
static void AITarget_19_c(Snes *snes) {
    // lda #0
    snes->cpu->a = 0;
    snes->cpu->z = true;
    snes->cpu->n = false;

    // jmp TargetMonsterTypeAll
    target_monster_type_all_emu(snes);
}

// PITFALLS: None. Simple register set and jump.
// HELPERS: target_monster_type_all_emu(snes) — delegates TargetMonsterTypeAll @ $B9:3D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB9
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AITarget_19 ($B9:38)