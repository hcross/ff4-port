// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DC, DP=0
// Logic: 
// 1. Generates a random number between 0 and 5 (inclusive) using RandXA.
// 2. Performs a right-rotate on RAM[$28A4] for the number of times determined by the random value.
// 3. If the random value was exactly 5, jumps to SetMagicStatus2.
// 4. Otherwise, jumps to SleepParalyzeEffect.
static void MagicEffect_20_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Prepare inputs for RandXA: X = max, A = min
    snes->cpu->x = 3;
    snes->cpu->a = 5;
    uint8_t rand_val = randxa_emu(snes); // jsr RandXA

    uint8_t y = rand_val;               // tay
    ram[0x00A9] = rand_val;             // sta a:$00a9 (Direct Page/Absolute access)

    // Rotate loop
    snes->cpu->c = true;                // sec
    for (int i = y; i > 0; i--) {       // dey / bne @dca8
        uint8_t val = ram[0x28A4];
        bool carry = snes->cpu->c;
        
        // ROR: shift right, carry into bit 7, bit 0 into carry
        snes->cpu->c = (val & 1);
        ram[0x28A4] = (uint8_t)((val >> 1) | (carry << 7));
    }

    // Final jump logic
    if (rand_val == 0x05) {             // lda a:$00a9 / cmp #$05 / beq @dcb8
        set_magic_status2_emu(snes);    // jmp SetMagicStatus2
    } else {
        sleep_paralyze_effect_emu(snes); // jmp SleepParalyzeEffect
    }
}

// PITFALLS: 7 (8-bit truncation on shift/rotate), 1 (DB=$DC used for RAM access)
// HELPERS: randxa_emu, set_magic_status2_emu, sleep_paralyze_effect_emu
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x28A4=1
//   output_ram:  0x28A4=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDC
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_20 ($DC:9B)