// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: 
//   1. Load 8-bit value from $269D into X.
//   2. Store X as 16-bit value at $3902 (effectively zero-extending $269D).
//   3. Left shift the 16-bit value at $3902 (value * 2).
//   4. Jump to CalcDmg for final damage processing.
static void MagicEffect_2b_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $269d / tax / stx $3902
    // X is 16-bit, so this writes 0x00 to $3903 and value to $3902.
    uint8_t val = ram[0x269D];
    write16(ram, 0x3902, (uint16_t)val);

    // asl $3902 / rol $3903
    // This is a 16-bit shift left of the value at $3902.
    uint16_t shifted = read16(ram, 0x3902) << 1;
    write16(ram, 0x3902, shifted);

    // jmp CalcDmg
    calc_dmg_emu(snes);
}

// PITFALLS: 7 (Shift truncation: 16-bit shift used here as $3902 is treated as a word),
//           8 (Inherited mode: mf=true for battle code)
// HELPERS: calc_dmg_emu(snes) — delegates CalcDmg @ $03:C99F
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x269D=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Jumps to CalcDmg, parity depends on that routine's output)

REVERSED_FUNCTION: battle::MagicEffect_2b ($DD:C9)