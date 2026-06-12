// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   Clears A and X (via clr_ax), loads 98 into A, and calls RandXA.
//   RandXA is expected to return a random value between 0 and A (inclusive).
static uint16_t Rand99_c(Snes *snes) {
    // clr_ax: A = X = DP (DP=0 in battle module)
    snes->cpu->a = 0;
    snes->cpu->x = 0;

    // lda #98
    snes->cpu->a = 98;
    snes->cpu->z = (snes->cpu->a == 0);
    snes->cpu->n = (snes->cpu->a & 0x80) != 0;

    // jsr RandXA
    return rand_xa_emu(snes);
}

// PITFALLS: 5 (clr_ax is tdc/tax, effectively clearing A and X when DP=0)
// HELPERS: rand_xa_emu(snes) — delegates RandXA @ $83:79
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (returns value in A via emulator)

REVERSED_FUNCTION: battle::Rand99 ($85:8B)