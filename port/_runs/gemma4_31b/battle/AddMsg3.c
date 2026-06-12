// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine triggers the display of a specific battle message by
// writing constants to the text system control bytes.
static void AddMsg3_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x33C8] = 0xF8; // lda #$f8 / sta $33c8 (display text flag)
    ram[0x33C9] = 0x03; // lda #$03 / sta $33c9 (message index 3)
}

// PITFALLS: 1 (DB=$7E required for WRAM access at $33C8/9)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x33C8=1, 0x33C9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AddMsg3 ($85:B1)