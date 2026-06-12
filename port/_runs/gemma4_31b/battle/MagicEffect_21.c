// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   Calculates a random value using RandXA (with 1 as upper bound).
//   If result is 0: sets status $28A3 = 0x80 and jumps to SetMagicStatus2.
//   If result is > 0: sets status $28A4 = 0x20 and jumps to SleepParalyzeEffect.
static void MagicEffect_21_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // ldx #0 / lda #1 / jsr RandXA
    snes->cpu->x = 0;
    snes->cpu->a = 1;
    uint16_t rand_res = randxa_emu(snes); // result returned in A

    // tax / bne @dcce
    // Since we are in mf=true (8-bit A), bne checks the low byte
    if (rand_res == 0) {
        // @dcbb path (result is 0)
        ram[0x28A3] = 0x80;
        set_magic_status2_emu(snes);
    } else {
        // @dcce path (result is non-zero)
        ram[0x28A4] = 0x20;
        sleep_paralyze_effect_emu(snes);
    }
}

// PITFALLS: None specific to this logic, but used mf=true for battle convention.
// HELPERS: randxa_emu, set_magic_status2_emu, sleep_paralyze_effect_emu
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x28A3=1 or 0x28A4=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (routine ends in jumps to different sub-routines)

REVERSED_FUNCTION: battle::MagicEffect_21 ($DC:BB)