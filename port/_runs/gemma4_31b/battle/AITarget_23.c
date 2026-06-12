// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E (implied by battle convention), DP=0
// Logic: Sets target parameters in WRAM before jumping to RandAITarget.
//   - ram[0xAF] = 0
//   - ram[0xB0] = 12 (0x0C)
//   - ram[0xAD] = ram[0xD2]
//   - jump to rand_ai_target_emu(snes)
static void AITarget_23_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xAF] = 0x00;
    ram[0xB0] = 0x0C;
    ram[0xAD] = ram[0xD2];

    // JMP is treated as a call here as it's the final operation of the routine
    rand_ai_target_emu(snes);
}

// PITFALLS: None relevant for this routine.
// HELPERS: rand_ai_target_emu(snes) — delegates RandAITarget @ $BA:9C
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x00D2=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AITarget_23 ($BA:EF)