// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Sets a target monster type constant (0x02) into RAM and jumps to
// the monster type target selection logic.
static void AICondTarget_27_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xAB] = 0x02;                    // lda #$02 / sta $ab
    target_monster_type_emu(snes);      // jmp TargetMonsterType (delegated)
}

// PITFALLS: 1 (DB=$7E required for RAM access)
// HELPERS: target_monster_type_emu(snes) — delegates TargetMonsterType @$C0:BFFB
//
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xAB=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICondTarget_27 ($C0:42)