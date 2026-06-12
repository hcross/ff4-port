// This routine acts as a dispatcher to determine a valid target.
// It first attempts to target a character, and if that fails/returns,
// it jumps to the monster targeting logic.
static void AnyTarget_c(Snes *snes) {
    // The routine is a simple sequence of a call and a jump.
    // TargetCharacter is called via JSR, meaning it returns here.
    target_character_emu(snes);
    
    // TargetMonster is called via JMP, meaning it does not return to this routine.
    target_monster_emu(snes);
}

// PITFALLS: None. This is a simple dispatch routine.
// HELPERS: target_character_emu(snes) - delegates TargetCharacter @ $BF:C3
//          target_monster_emu(snes)    - delegates TargetMonster @ $C0:61
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: auto
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AnyTarget ($C0:49)