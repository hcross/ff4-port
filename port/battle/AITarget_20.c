// Sets up monster targeting flags for status-based targeting
// and jumps to GetMonsterWithStatus to select a monster.
static void AITarget_20_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xAD] = 0x08;  // Targeting flag for status condition
    ram[0xAE] = 0;
    ram[0xAF] = 0;
    ram[0xB0] = 0;
    get_monster_with_status_emu(snes);  // jmp GetMonsterWithStatus
}

// PITFALLS: 1 (DB must be $7E for WRAM writes)
// HELPERS: get_monster_with_status_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xad=1, 0xae=1, 0xaf=1, 0xb0=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_20 ($BA:74)