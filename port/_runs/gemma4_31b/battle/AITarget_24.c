// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine initializes target parameters for the AI and jumps
// to the random target selection logic.
//   Inputs: None
//   Outputs: ram[$AF], ram[$B0], ram[$AD] and the result of RandAITarget
static void AITarget_24_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xAF] = 0x05; // lda #$05 / sta $af
    ram[0xB0] = 0x0C; // lda #$0c / sta $b0
    ram[0xAD] = 0xFF; // lda #$ff / sta $ad

    rand_ai_target_emu(snes); // jmp RandAITarget
}

// PITFALLS: 1 (DB=$7E assumed for battle module WRAM access)
// HELPERS: rand_ai_target_emu(snes) — delegates RandAITarget @ $BA:9C
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Jump to another routine; parity depends on RandAITarget's side effects)

REVERSED_FUNCTION: battle::AITarget_24 ($BA:FE)