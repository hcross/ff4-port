// Sets the magic effect #25: heal party to max HP
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: no register inputs
// Output: $a4-$a5 = capped hp value (16-bit), $38e6 = 5, $2683 = 0x80
static void MagicEffect_25_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x38E6] = 5;                    // sta $38e6
    uint16_t hp = read16(ram, 0x2687);  // longa then lda $2687
    if (hp > 9999) hp = 9999;           // cmp #9999 / bcc / lda #9999
    write16(ram, 0xA4, hp);             // sta $a4 (16-bit)
    ram[0x2683] = 0x80;                 // shorta0 / lda #$80 / sta $2683
}

// PITFALLS: 1 (DB=$7E for 16-bit absolute access), 6 (mode A 16-bit for read16)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2687=2
//   output_ram:  0xa4=2, 0x38e6=1, 0x2683=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_25 ($DD:1D)