// Sets up monster targeting flags for status check and jumps to GetMonsterWithStatus.
// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
//   No input registers used.
//   Outputs: ram[$AD] = 0x20, ram[$AE..$B0] = 0
static void AITarget_1e_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xAD] = 0x20;  // lda #$20 / sta $ad
    ram[0xAE] = 0;     // stz $ae
    ram[0xAF] = 0;     // stz $af
    ram[0xB0] = 0;     // stz $b0
    get_monster_with_status_emu(snes);  // jmp GetMonsterWithStatus
}

// PITFALLS: 1 (DB must be $7E for WRAM writes)
// HELPERS: get_monster_with_status_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xAD=1, 0xAE=1, 0xAF=1, 0xB0=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_1e ($BA:0004)