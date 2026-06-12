// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Sets bit 6 (0x40) of the flag byte at $34C2.
static void Cmd_21_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x34C2] |= 0x40;
}

// PITFALLS: None. Simple 8-bit OR operation.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x34C2=1
//   output_ram:  0x34C2=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Cmd_21 ($B3:3F)