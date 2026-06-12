// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This function manipulates HP of a target based on a random value.
// It compares a random number (0-9) + 1 against a target's HP and either
// sets the HP to that value (if higher) or removes the target (if lower).
static void MagicEffect_03_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t rand_val = rand_xa_emu(snes, 1, 9); // jsr RandXA with X=1, A=9
    ram[0xA9] = rand_val;                       // stx $a9 (X 16-bit, but only low byte used)

    uint16_t target_hp = read16(ram, 0x2707);   // longa / lda $2707
    uint16_t rand_hp = (uint16_t)rand_val;      // lda $a9 (now 16-bit)
    if (target_hp < rand_hp) {                  // cmp $a9 / bcc (inverted!)
        write16(ram, 0x2707, rand_hp);          // lda $a9 / sta $2707
    } else {
        remove_target_emu(snes);                // jsr RemoveTarget
    }
}

// PITFALLS: 1 (DB=$7E required for 16-bit absolute access),
//           3 (CMP/BCC inversion: bcc branches when A<C, so we enter body when A>=C),
//           6 (Mode A/X tracking: routine switches from 8-bit A to 16-bit A)
// HELPERS: rand_xa_emu(snes, x, a) — delegates RandXA @ $03:8379,
//          remove_target_emu(snes) — delegates RemoveTarget @ $00:E030
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x2707=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_03 ($D4:43)