// Sets up a call to GetMonsterWithStatus by initializing $ad-$b0 to $00010000.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
static void AITarget_21_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xAD] = 0;
    ram[0xAE] = 1;
    ram[0xAF] = 0;
    ram[0xB0] = 0;
    GetMonsterWithStatus_emu(snes); // jmp GetMonsterWithStatus
}

// PITFALLS: 1 (DB must be $7E for WRAM access)
// HELPERS: GetMonsterWithStatus_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_21 ($BA:81)