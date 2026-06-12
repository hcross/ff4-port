// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: Sets the status filter to 0x08 and calls GetMonsterWithStatus 
// to find a monster with that specific status.
static void AITarget_20_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Set status filter: $AD = 0x08, $AE..$B0 = 0
    ram[0xAD] = 0x08;
    ram[0xAE] = 0;
    ram[0xAF] = 0;
    ram[0xB0] = 0;

    // Jump to GetMonsterWithStatus (delegated)
    get_monster_with_status_emu(snes);
}

// PITFALLS: 1 (DB=$7E required for WRAM access)
// HELPERS: get_monster_with_status_emu(snes) — delegates GetMonsterWithStatus @ $BA:11
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AITarget_20 ($BA:74)