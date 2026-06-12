// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This function sets $28A4 to $20, calls SleepParalyzeEffect, then jumps to _d3e4.
static void MagicEffect_23_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x28A4] = 0x20;                  // lda #$20 / sta $28a4
    sleep_paralyze_effect_emu(snes);     // jsr SleepParalyzeEffect
    _d3e4_emu(snes);                     // jmp _d3e4
}

// PITFALLS: 1 (DB must be $7E for WRAM writes), 2 (no flag setup needed — no conditional entry)
// HELPERS: sleep_paralyze_effect_emu(snes), _d3e4_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_23 ($DC:FB)