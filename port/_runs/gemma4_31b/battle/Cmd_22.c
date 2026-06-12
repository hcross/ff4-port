// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine simply sets a specific flag/value in WRAM to 0xFF.
static void Cmd_22_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x357C] = 0xFF;
}

// PITFALLS: None relevant for this simple store.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x357C=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::Cmd_22 ($E4:3B)