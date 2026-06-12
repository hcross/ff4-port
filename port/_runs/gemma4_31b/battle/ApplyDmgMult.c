// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   If mult == 0: damage = 0
//   If mult == 1: damage = damage / 2
//   If mult > 1:  damage = (mult / 2) * damage
//
// Note: This routine starts with a conditional branch (bne), meaning
// the Z/N flags must be set by the caller based on the multiplier value.
static void ApplyDmgMult_c(Snes *snes, uint8_t mult) {
    uint8_t *ram = snes->ram;

    if (mult == 0) {                     // bne @ca48 -> not taken
        ram[0xA4] = 0;
        ram[0xA5] = 0;
        return;
    }

    uint8_t shifted = (uint8_t)(mult >> 1); // lsr A (Pitfall 7: truncate to 8-bit)
    if (shifted == 0) {                  // bne @ca50 -> not taken (mult was 1)
        uint16_t dmg = read16(ram, 0xA4);
        dmg >>= 1;                       // lsr $a5 / ror $a4
        write16(ram, 0xA4, dmg);
        return;
    }

    // mult > 1: Perform 16-bit multiplication
    // tax / stx $393d: X is 16-bit, high byte is preserved from 'B' (Pitfall 9)
    // However, in this specific flow, 'mult' is 8-bit and we only need the shifted value.
    // To ensure parity with a zero-extended X, we cast to uint16_t.
    write16(ram, 0x393D, (uint16_t)shifted); 
    
    uint16_t dmg = read16(ram, 0xA4);
    write16(ram, 0x393F, dmg);           // ldx $a4 / stx $393f
    
    mult16_emu(snes);                    // jsr Mult16 (delegated)
    
    write16(ram, 0xA4, read16(ram, 0x3941)); // ldx $3941 / stx $a4
}

// PITFALLS: 1 (DB=$7E), 2 (Caller must set Z/N for first bne), 
//           7 (8-bit shift truncation), 9 (X-register extension)
// HELPERS: mult16_emu(snes) — delegates Mult16 @ $03:83B9
//          read16/write16 — LE 16-bit accessors

// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x00A4=2
//   output_ram:  0x00A4=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::ApplyDmgMult ($CA:41)