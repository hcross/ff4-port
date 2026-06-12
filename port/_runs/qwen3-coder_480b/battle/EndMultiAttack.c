// Scans through $33C2 for a #$FF or #$FC marker.
// If #$FC is found, increments $a9 and replaces the sequence with [$E1, $FC, $FF].
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
static void EndMultiAttack_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = 0;                      // clr_ax → X = 0 (A also 0, but unused)
    ram[0xA9] = 0;                       // stx $a9 (low byte of X = 0)

    for (;;) {
        uint8_t a = ram[0x33C2 + x];     // lda $33c2,x
        if (a == 0xFF) break;            // cmp #$ff / beq @b36b → exit loop
        if (a == 0xFC) {                 // cmp #$fc / beq @b35a → handle marker
            ram[0xA9]++;                 // inc $a9
            ram[0x33C2 + x] = 0xE1;      // lda #$e1 / sta $33c2,x
            ram[0x33C2 + x + 1] = 0xFC;  // lda #$fc / sta $33c3
            ram[0x33C2 + x + 2] = 0xFF;  // lda #$ff / sta $33c4
            break;
        }
        x++;                             // inx
    }
    // rts → return
}

// PITFALLS: 1 (DB=$7E assumed), 8 (clr_ax is tdc/tax, not lda #0/ldx #0)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x33C2=1
//   output_ram:  0xA9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::EndMultiAttack ($B3:48)