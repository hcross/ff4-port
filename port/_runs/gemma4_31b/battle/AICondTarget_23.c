// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C0, DP=0
// This routine is a simple sequence: it attempts to target a monster,
// then jumps to the 'NoSelfTarget' logic if the targeting process continues.
// Since NoSelfTarget is a 'jmp' (tail-call), it is implemented as a function call.
static void AICondTarget_23_c(Snes *snes) {
    // jsr TargetMonster
    target_monster_emu(snes);

    // jmp NoSelfTarget
    no_self_target_emu(snes);
}

// PITFALLS: None relevant for this simple sequence.
// HELPERS: 
//   target_monster_emu(snes) — delegates TargetMonster @ $C0:61
//   no_self_target_emu(snes)  — delegates NoSelfTarget @ $C0:52
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AICondTarget_23 ($C0:6C)