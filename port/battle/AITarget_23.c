// Sets up AI target selection for a specific pattern:
//   $af-$b0 = $000c (target offset/index)
//   $ad = $d2 (copies enemy index)
// Then jumps to RandAITarget to finalize selection.
static void AITarget_23_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xAF] = 0x00;
    ram[0xB0] = 0x0c;
    ram[0xAD] = ram[0xD2];
    rand_ai_target_emu(snes);  // jmp RandAITarget
}

// PITFALLS: 1 (DB must be $7E for WRAM access)
// HELPERS: rand_ai_target_emu(snes) — delegates RandAITarget @ $BA:9C
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xd2=1
//   output_ram:  0xad=1, 0xaf=1, 0xb0=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_23 ($BA:EF)