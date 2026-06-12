// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Sets the battle message display flags in WRAM.
//   $33C6 = 0xF8 (Display text signal)
//   $33C7 = 0x03 (Battle message type)
static void AddMsg2_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x33C6] = 0xF8; // lda #$f8 / sta $33c6
    ram[0x33C7] = 0x03; // lda #$03 / sta $33c7
}

// PITFALLS: None. Simple memory writes.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x33C6=1, 0x33C7=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::AddMsg2 ($85:A6)