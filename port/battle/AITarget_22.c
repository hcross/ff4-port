// Sets up AI target selection with fixed values and jumps to RandAITarget.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// No input registers; all state communicated via RAM.
static void AITarget_22_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xB0] = 0x0C;
    ram[0xAF] = 0x00;
    ram[0xAD] = 0xFF;  // dec on A=0 yields 0xFF (8-bit)
    rand_ai_target_emu(snes);  // jmp RandAITarget
}

// PITFALLS: 1 (DB must be $7E for writes to $B0/$AF/$AD to reach WRAM)
// HELPERS: rand_ai_target_emu(snes) — delegates RandAITarget @ $BA:9C
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_22 ($BA:8E)