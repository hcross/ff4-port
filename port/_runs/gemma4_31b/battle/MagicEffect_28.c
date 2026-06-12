// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0 (battle convention)
// This routine is a sequential bridge: it applies a damage effect and then
// jumps to set a specific magic status.
static void MagicEffect_28_c(Snes *snes) {
    // jsr MagicDmgEffect
    magic_dmg_effect_emu(snes);
    
    // jmp SetMagicStatus2 (Tail-call optimization: direct jump)
    set_magic_status2_emu(snes);
}

// PITFALLS: None applicable (simple sequence of calls).
// HELPERS: 
//   magic_dmg_effect_emu(snes) -> delegates MagicDmgEffect @$D378
//   set_magic_status2_emu(snes) -> delegates SetMagicStatus2 @$D514
//
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (Tail-call jump prevents single output RAM observation)

REVERSED_FUNCTION: battle::MagicEffect_28 ($DD:95)