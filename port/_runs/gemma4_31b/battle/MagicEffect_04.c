// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   If ram[0xCD] == ram[0xCE], skip to end.
//   Otherwise, calculate damage (CalcDmg).
//   The resulting damage (low byte in $a4) is stored in $a2.
//   If ram[0x2740] bit 7 is set, set bit 7 of the damage high byte ($a5).
//   Else, set bit 7 of the damage result modifier ($a3).
static void MagicEffect_04_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0xCD] == ram[0xCE]) { // lda $cd / cmp $ce / beq @d487
        return;
    }

    calc_dmg_emu(snes);           // jsr CalcDmg

    // CalcDmg results: $a4 = dmg_lo, $a5 = dmg_hi, $a3 = modifier
    uint8_t dmg_lo = ram[0xA4];
    ram[0xA2] = dmg_lo;           // ldx $a4 / stx $a2 (X is 16-bit, but only lo byte used)

    if (ram[0x2740] & 0x80) {     // lda $2740 / and #$80 / beq @d481
        ram[0xA5] |= 0x80;        // lda $a5 / ora #$80 / sta $a5
    } else {
        ram[0xA3] |= 0x80;        // lda $a3 / ora #$80 / sta $a3
    }
}

// PITFALLS: 1 (DB=$7E for WRAM access), 8 (mf=true assumed for battle effect)
// HELPERS: calc_dmg_emu(snes) — delegates CalcDmg @ $C99F
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xCD=1, 0xCE=1, 0x2740=1
//   output_ram:  0xA2=1, 0xA3=1, 0xA5=1 (depends on CalcDmg output)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_04 ($D4:66)