// Sets up a 16-bit little-endian value $AF:$AE = $8000, then jumps to GetMonsterWithStatus.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0) — inherited from battle module convention.
// No input registers are used. No conditional branches at entry.
static void AITarget_29_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xAD] = 0;
    ram[0xAE] = 0;
    ram[0xAF] = 0x80;
    ram[0xB0] = 0x80;
    get_monster_with_status_emu(snes);  // jmp GetMonsterWithStatus
}

// PITFALLS: 1 (DB must be $7E for WRAM access — handled by emulator on jsr)
// HELPERS: get_monster_with_status_emu(snes) — delegates GetMonsterWithStatus @ $00:BA11
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xAD=1, 0xAE=1, 0xAF=1, 0xB0=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_29 ($BB:006B)