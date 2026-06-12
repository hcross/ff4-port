// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Sets up selection parameters for RandAITarget:
// - Target range max ($B0) = 12
// - Target range min ($AF) = 0
// - Loop iterations  ($AD) = 255 (0x00 DEC)
static void AITarget_22_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xB0] = 0x0C; // lda #$0c / sta $b0
    ram[0xAF] = 0x00; // lda #$00 / sta $af
    
    // lda #$00 / dec / sta $ad
    // Pitfall 7: In 8-bit mode, 0x00 - 1 = 0xFF
    ram[0xAD] = 0xFF;

    RandAITarget_emu(snes); // jmp RandAITarget
}

// PITFALLS: 7 (Arithmetic truncation: dec 0x00 in 8-bit mode results in 0xFF)
// HELPERS: RandAITarget_emu(snes) — delegates RandAITarget @ $BA:9C
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xAD=1, 0xAF=1, 0xB0=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_22 ($BA:8E)