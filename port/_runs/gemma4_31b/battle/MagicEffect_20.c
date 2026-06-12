// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DC, DP=0
// Logic:
// 1. Generates a random value using RandXA (X=3, A=5).
// 2. Stores the result in RAM[0x00A9] and uses it as a loop counter.
// 3. Performs the random value number of Right-Rotates (ROR) on RAM[0x28A4].
// 4. Depending on whether the random value was 5, jumps to SetMagicStatus2 or SleepParalyzeEffect.
static void MagicEffect_20_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // Prepare inputs for RandXA: X = max, A = min
    cpu->x = 3;
    cpu->a = 5;
    RandXA_emu(snes);
    
    uint8_t rand_val = (uint8_t)cpu->a; // Result of RandXA is in A
    uint8_t y = rand_val;               // tay
    ram[0x00A9] = rand_val;             // sta a:$00a9

    cpu->c = true;                      // sec
    for (int i = y; i > 0; i--) {       // dey / bne @dca8
        uint8_t val = ram[0x28A4];
        bool carry = cpu->c;
        
        // ROR: bit 0 moves to carry, carry moves to bit 7, others shift right
        cpu->c = (val & 1) != 0;
        ram[0x28A4] = (uint8_t)((val >> 1) | (carry ? 0x80 : 0x00)); // Pitfall 7
    }

    // Final jump logic based on stored random value
    if (ram[0x00A9] == 0x05) {          // lda a:$00a9 / cmp #$05 / beq @dcb8
        SetMagicStatus2_emu(snes);      // jmp SetMagicStatus2
    } else {
        SleepParalyzeEffect_emu(snes);   // jmp SleepParalyzeEffect
    }
}

// PITFALLS: 1 (DB=$DC for RAM access), 7 (8-bit truncation/rotation)
// HELPERS: RandXA_emu(snes), SleepParalyzeEffect_emu(snes), SetMagicStatus2_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x28A4=1
//   output_ram:  0x28A4=1, 0x00A9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDC
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_20 ($DC:9B)