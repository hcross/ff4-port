// This function sets A to 2 and jumps to TargetMonsterTypeAll.
// Entry mode: A 8-bit (inherited from caller), DB=$7E, DP=0
// Input: none (hardcoded A=2)
// Output: none (tail call to TargetMonsterTypeAll)
static void AITarget_1b_c(Snes *snes) {
    // lda #2
    snes->cpu->a = 2;
    snes->cpu->z = (2 == 0);
    snes->cpu->n = (2 & 0x80) != 0;

    // jmp TargetMonsterTypeAll
    target_monster_type_all_emu(snes);
}

// PITFALLS: 1 (DB must be $7E for battle routines), 2 (flags Z/N set to match hardcoded A=2)
// HELPERS: target_monster_type_all_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=false, n=false
REVERSED_FUNCTION: battle::AITarget_1b ($B9:69)