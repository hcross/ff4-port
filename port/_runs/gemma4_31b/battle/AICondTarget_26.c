// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine sets a target monster type flag (0x01) and jumps to 
// the logic that selects a target based on that monster type.
static void AICondTarget_26_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    ram[0xAB] = 0x01; // lda #$01 / sta $ab
    
    target_monster_type_emu(snes); // jmp TargetMonsterType
}

// PITFALLS: None.
// HELPERS: target_monster_type_emu(snes) — delegates TargetMonsterType @$BFFB
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xAB=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::AICondTarget_26 ($C0:3B)