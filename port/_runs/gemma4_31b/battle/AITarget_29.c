// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine initializes the target status mask before jumping to the
// monster search routine. It sets a 16-bit value (0x8080) across RAM $AF-$B0.
static void AITarget_29_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xAD] = 0; // stz $ad
    ram[0xAE] = 0; // stz $ae
    
    // lda #$80 / sta $af / sta $b0
    ram[0xAF] = 0x80;
    ram[0xB0] = 0x80;

    // jmp GetMonsterWithStatus
    // Note: This is a tail-call. The parity harness expects the 
    // emulated function to handle the subsequent logic.
    get_monster_with_status_emu(snes);
}

// PITFALLS: 1 (DB=$7E required for WRAM access)
// HELPERS: get_monster_with_status_emu(snes) — delegates GetMonsterWithStatus @$BA11
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xAD=1, 0xAE=1, 0xAF=1, 0xB0=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::AITarget_29 ($BB:6B)