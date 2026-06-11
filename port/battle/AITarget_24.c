// This function sets up AI targeting parameters and jumps to RandAITarget.
// It writes fixed values to $af, $b0, and $ad, then delegates to RandAITarget.
static void AITarget_24_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xaf] = 0x05;
    ram[0xb0] = 0x0c;
    ram[0xad] = 0xff;
    RandAITarget_emu(snes);  // jmp RandAITarget
}

// PITFALLS: 1 (DB must be $7E for writes to $af/$b0/$ad to target WRAM)
// HELPERS: RandAITarget_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_24 ($BA:FE)