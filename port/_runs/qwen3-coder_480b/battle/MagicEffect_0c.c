// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM:
//   in : ram[$272B], ram[$28A3], ram[$2703]
//   out: ram[$2703] (modified in one path), or RemoveTarget called
static void MagicEffect_0c_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a;

    a = ram[0x272B] & ram[0x28A3];       // lda $272b / and $28a3
    if (a != 0) goto remove_target;      // bne @d832

    a = ram[0x2703] & ram[0x28A3];       // lda $2703 / and $28a3
    if (a != 0) goto swap;               // bne @d835

    if (ram[0x2703] >= ram[0x28A3])      // cmp $28a3 / bcc @d835 (inverted)
        goto swap;

remove_target:
    remove_target_emu(snes);             // jmp RemoveTarget
    return;

swap:
    ram[0x2703] ^= ram[0x28A3];          // lda $2703 / eor $28a3 / sta $2703
}

// PITFALLS: 1 (DB=$7E required for RemoveTarget), 3 (CMP/BCC inversion)
// HELPERS: remove_target_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x272B=1, 0x28A3=1, 0x2703=1
//   output_ram:  0x2703=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_0c ($D8:1A)