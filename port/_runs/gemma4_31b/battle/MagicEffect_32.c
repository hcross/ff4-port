// This routine sets a specific magic effect parameter to 4.
// The instruction 'sta a:$00a8' uses absolute indexed addressing
// with the accumulator, targeting a fixed address in WRAM.
static void MagicEffect_32_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    // Entry mode: A is 8-bit (mf=true) based on battle module convention.
    // Absolute address $00A8 in battle module context (DB=$7E) targets $7E:00A8.
    ram[0x00A8] = 0x04;
}

// PITFALLS: 1 (DB=$7E is implicit for battle module absolute addresses)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x00A8=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_32 ($DF:D2)