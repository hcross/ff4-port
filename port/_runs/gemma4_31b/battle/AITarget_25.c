// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   If ram[0x29CD] == 1, then ram[0x29CD] becomes 0 and the turn is skipped.
//   If ram[0x29CD] != 1, then ram[0x29CD] is decremented, 
//   target parameters are set in RAM (0xAD-0xB0), and a random target is selected.
static void AITarget_25_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t val = ram[0x29CD];
    val--; // dec
    ram[0x29CD] = val;

    if (val == 0) { // bne @bb16 (not taken if val == 0)
        skip_ai_turn_emu(snes); // jmp SkipAITurn
        return;
    }

    // @bb16
    ram[0xAF] = 0x05; // sta $af
    ram[0xB0] = 0x0C; // sta $b0
    ram[0xAD] = ram[0xD2]; // lda $d2 / sta $ad
    rand_ai_target_emu(snes); // jmp RandAITarget
}

// PITFALLS: 7 (8-bit dec truncation: uint8_t handles this naturally)
// HELPERS: skip_ai_turn_emu(snes), rand_ai_target_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x29CD=1, 0x00D2=1
//   output_ram:  0x29CD=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (contains jumps to other logic flows)

REVERSED_FUNCTION: battle::AITarget_25 ($BB:0D)