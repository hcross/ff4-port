// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: Sets a target status value (0x10) and clears the following
// memory bytes before jumping to GetMonsterWithStatus.
static void AITarget_1f_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xAD] = 0x10; // lda #$10 / sta $ad
    ram[0xAE] = 0;    // stz $ae
    ram[0xAF] = 0;    // stz $af
    ram[0xB0] = 0;    // stz $b0

    get_monster_with_status_emu(snes); // jmp GetMonsterWithStatus
}

// PITFALLS: 1 (DB=$7E required for WRAM access)
// HELPERS: get_monster_with_status_emu(snes) — delegates GetMonsterWithStatus @ $BA:11
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xAD=1, 0xAE=1, 0xAF=1, 0xB0=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_1f ($BA:67)