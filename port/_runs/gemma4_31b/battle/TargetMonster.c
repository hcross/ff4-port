// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine initializes a constant value in WRAM (0x0D05) and jumps
// to the monster target selection logic at $BFC9.
static void TargetMonster_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xA9] = 0x05; // lda #$05 / sta $a9
    ram[0xAB] = 0x0D; // lda #$0d / sta $ab

    // jmp _bfc9 (delegated)
    // Since this is a jump to a functional routine, we call its emulated version.
    _bfc9_emu(snes);
}

// PITFALLS: 1 (DB=$7E assumed for battle module)
// HELPERS: _bfc9_emu(snes) — delegates logic at $BFC9
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xAB=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::TargetMonster ($C0:61)