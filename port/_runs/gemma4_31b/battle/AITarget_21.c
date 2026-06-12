// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   This routine initializes a status search filter:
//   $AD = 0 (likely a status bitmask or index)
//   $AE = 1 (likely a target status ID)
//   $AF = 0 (additional filter/parameter)
//   $B0 = 0 (additional filter/parameter)
//   Then jumps to GetMonsterWithStatus to find a monster matching these criteria.
static void AITarget_21_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xAD] = 0;
    ram[0xAE] = 1;
    ram[0xAF] = 0;
    ram[0xB0] = 0;

    get_monster_with_status_emu(snes);
}

// PITFALLS: 1 (DB=$7E for WRAM access)
// HELPERS: get_monster_with_status_emu(snes) — delegates GetMonsterWithStatus @ $BA:11
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Routine is a setup wrapper for another function, output depends on GetMonsterWithStatus)

REVERSED_FUNCTION: battle::AITarget_21 ($BA:81)