// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DC, DP=0
// Purpose: Implements a specific magic effect (22).
//   1. Calls RandXA(2) to get a value 0 or 1.
//   2. If 0: set $28A4 = 0x20 and jump to SleepParalyzeEffect.
//   3. If 1: dec A -> 0, bne is false, set $28A3 = 0x04 and jump to SetMagicStatus2.
static void MagicEffect_22_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    snes->cpu->x = 0;
    snes->cpu->a = 2;
    RandXA_emu(snes); // Returns value in cpu->a
    
    uint8_t a = (uint8_t)snes->cpu->a; // Pitfall 6: Mode A 8-bit
    
    if (a == 0) { // tax / bne @dce9 (not taken)
        ram[0x28A4] = 0x20;
        SleepParalyzeEffect_emu(snes);
        return;
    }

    // @dce9
    a--; // dec A (Pitfall 7: 8-bit truncation)
    if (a != 0) { // bne @dcf3
        ram[0x28A4] = 0x80; // @dcf3
    } else {
        ram[0x28A3] = 0x04; // @dce9 fallthrough/branch not taken
    }

    SetMagicStatus2_emu(snes); // @dcf8
}

// PITFALLS: 1 (DB=$DC), 6 (8-bit A), 7 (8-bit truncation on dec)
// HELPERS: RandXA_emu(snes), SleepParalyzeEffect_emu(snes), SetMagicStatus2_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x28A3=1 or 0x28A4=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDC
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_22 ($DC:D6)