// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine generates a random number between 0 and 254 (0xFE).
// It initializes the range parameters for RandXA and calls it.
//
// Logic:
//   A = 0 (via clr_ax/tdc)
//   X = 0 (via clr_ax/tax)
//   A = 0xFF (upper limit for RandXA)
//   Result = RandXA(0, 0xFF)
static uint16_t Rand_c(Snes *snes) {
    // clr_ax: A = 0, X = 0 (since DP=0)
    snes->cpu->a = 0;
    snes->cpu->x = 0;

    // lda #$ff
    snes->cpu->a = 0xFF;

    // jsr RandXA
    // RandXA usually takes A as the upper limit and X as the lower limit
    return randxa_emu(snes);
}

// PITFALLS: 5 (clr_ax is tdc/tax, acting as zero-clear when DP=0)
// HELPERS: randxa_emu(snes) — delegates RandXA @ $8379
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   custom_spike: yes (returns value in A register via randxa_emu)

REVERSED_FUNCTION: battle::Rand ($85:93)