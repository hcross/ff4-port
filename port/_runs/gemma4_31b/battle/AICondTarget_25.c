// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: Clears a target-related flag in WRAM ($AB) and jumps to the 
// TargetMonsterType routine to evaluate the target.
static void AICondTarget_25_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xAB] = 0; // stz $ab

    // jmp TargetMonsterType
    target_monster_type_emu(snes);
}

// PITFALLS: 1 (DB=$7E required for WRAM access)
// HELPERS: target_monster_type_emu(snes) — delegates TargetMonsterType @ $BF:FB
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xAB=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICondTarget_25 ($BF:F6)