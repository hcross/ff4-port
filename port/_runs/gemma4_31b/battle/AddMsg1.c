// This routine sets the message flags to display a battle message.
// It writes specific control bytes to the message system RAM.
static void AddMsg1_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x33C2] = 0xF8; // lda #$f8 / sta $33c2 (display text)
    ram[0x33C3] = 0x03; // lda #$03 / sta $33c3 (battle message)
}

// PITFALLS: None applicable. Simple absolute stores.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x33C2=1, 0x33C3=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::AddMsg1 ($00:00859B)