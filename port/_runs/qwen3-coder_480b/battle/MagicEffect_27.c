// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM (no register I/O — convention battle):
//   in : ram[$2709] 16-bit value
//   out: ram[$3945] = ram[$2709], ram[$3947] = 0x000A
static void MagicEffect_27_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x3945] = ram[0x2709];
    ram[0x3946] = ram[0x270A];
    write16(ram, 0x3947, 0x000A);
    // bra _dd77 → unconditional jump to next routine
    _dd77_emu(snes);
}

// PITFALLS: 1 (DB=$7E required for correct absolute addressing)
// HELPERS: _dd77_emu(snes) — delegates _dd77 (untranslated)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2709=2
//   output_ram:  0x3945=2, 0x3947=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_27 ($DD:51)