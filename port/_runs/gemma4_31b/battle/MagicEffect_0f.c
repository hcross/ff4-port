// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine updates the speed/action modifier for a target.
// It adds a modifier value to the current value and clamps the result
// between a minimum of 12 (0x0C) and a maximum of 32 (0x20).
static void MagicEffect_0f_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $273b / adc $289d
    // Pitfall 7: arithmetic in 8-bit mode must be truncated to uint8_t
    uint8_t val = (uint8_t)(ram[0x273B] + ram[0x289D]);

    if (val >= 0x20) {               // cmp #$20 / bcs @d89a
        val = 0x20;                  // lda #$20
    } else if (val >= 0x0C) {        // cmp #$0c / bcs @d89c
        // val remains as is, it is between 12 and 31
    } else {
        val = 0x0C;                  // lda #$0c
    }

    ram[0x273B] = val;               // sta $273b
}

// PITFALLS: 7 (8-bit addition truncated to uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x273B=1, 0x289D=1
//   output_ram:  0x273B=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_0f ($D8:87)