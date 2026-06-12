// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$85, DP=0
// This routine generates a random character (byte) by calling RandXA
// with X=0 and A=4. The result is returned in the accumulator.
static uint16_t RandChar_c(Snes *snes) {
    snes->cpu->x = 0;          // ldx #0
    snes->cpu->a = 4;          // lda #4
    
    // jsr RandXA (delegated)
    // RandXA typically returns the result in the accumulator
    return rand_xa_emu(snes);
}

// PITFALLS: None. This is a simple wrapper.
// HELPERS: rand_xa_emu(snes) — delegates RandXA @ $83:79
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x85
//   entry_flags: z=auto, n=auto
//   returns:     a=8-bit result

REVERSED_FUNCTION: battle::RandChar ($85:82)