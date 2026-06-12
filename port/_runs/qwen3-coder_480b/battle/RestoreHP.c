// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM (no register I/O — convention battle):
//   in : ram[$2740], ram[$352a], ram[$26d2], ram[$3906], ram[$2709], ram[$2707]
//   out: ram[$a4-$a5] = damage (signed 16-bit), with bit 15 set (negative)
static void RestoreHP_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Check if bit 7 of $2740 is set (negative flag)
    if (ram[0x2740] & 0x80) {
        // Jump to _d3ae (delegated)
        run_emulated_func(snes, 0xD3AE);
        return;
    }

    // Check if $352a != 0
    if (ram[0x352a] != 0) {
        goto calc_dmg;
    }

    // Check if $26d2 == 0x11
    if (ram[0x26d2] != 0x11) {
        goto calc_dmg;
    }

    // Check if $3906 == 0x01
    if (ram[0x3906] != 0x01) {
        goto calc_dmg;
    }

    // longa: A 16-bit mode
    // SEC + SBC to compute 16-bit difference
    uint16_t a2709 = read16(ram, 0x2709);
    uint16_t a2707 = read16(ram, 0x2707);
    uint16_t diff = a2709 - a2707;  // SEC makes this standard subtraction
    write16(ram, 0xA4, diff);
    // shorta0: back to A 8-bit mode
    goto set_sign;

calc_dmg:
    calc_dmg_emu(snes);  // JSR CalcDmg

set_sign:
    // Set bit 7 of $a5 (sign bit for 16-bit value)
    ram[0xA5] |= 0x80;
}

// PITFALLS: 1 (DB=$7E required for delegated _d3ae), 3 (CMP/BCS-style
// comparisons), 6 (mode A 8-bit vs 16-bit), 7 (arithmetic truncation in
// 8-bit mode — though this routine uses longa/shorta correctly)
// HELPERS: run_emulated_func(snes, 0xD3AE) for _d3ae,
//          calc_dmg_emu(snes) for CalcDmg @ $03:C99F
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x2740=1, 0x352a=1, 0x26d2=1, 0x3906=1, 0x2709=2, 0x2707=2
//   output_ram:  0xA4=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
REVERSED_FUNCTION: battle::RestoreHP ($D4:0C)