// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Generates a random number between 1 and 9.
// If the value at $2707 (16-bit) is >= this random number, $2707 is updated to the random number.
// Otherwise, the target is removed.
static void MagicEffect_03_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Setup for RandXA: X=1 (min), A=9 (max)
    snes->cpu->x = 1;
    snes->cpu->a = 9;
    RandXA_emu(snes); // jsr RandXA - result is in A

    // tax / stx $a9
    uint8_t random_val = (uint8_t)snes->cpu->a;
    ram[0xA9] = random_val;

    // longa / lda $2707
    uint16_t val_2707 = read16(ram, 0x2707);

    // cmp $a9 / bcc @d45f
    // Pitfall 3: BCC branches if A < ram[$a9].
    if (val_2707 < random_val) {
        // @d45f: shorta0 / jsr RemoveTarget
        RemoveTarget_emu(snes);
    } else {
        // lda $a9 / sta $2707 (shorta)
        // Note: sta $2707 in 8-bit mode only writes the low byte.
        ram[0x2707] = random_val;
    }
}

// PITFALLS: 3 (CMP/BCC inversion), 6 (Mode A transition), 1 (DB=$7E)
// HELPERS: RandXA_emu(snes), RemoveTarget_emu(snes), read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2707=2, 0x00A9=1
//   output_ram:  0x2707=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_03 ($D4:43)