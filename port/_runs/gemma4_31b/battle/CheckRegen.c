// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FF, DP=0
// Logic:
//   Iterates through 5 characters (Y=5 down to 1).
//   Checks if character at offset (X * $20) is Fusoya (type == 0x13).
//   If Fusoya, checks status flags:
//     - If Dead or Stone (0xC0 mask), regen is disabled.
//     - If Paralyzed, Sleep, Charm, or Berserk (0x3C mask), regen is disabled.
//   If not Fusoya, the character is skipped by incrementing X by 0x80.
//
// Note: The routine uses $2000 as a base for character data relative to X.
// In DB=$FF, $2000 refers to an address in the current data bank.
static void CheckRegen_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = 0; // clr_ax (TDC/TAX with DP=0)
    uint8_t y = 5;  // ldy #5

    while (y > 0) {
        // a = ram[DB:$2000 + x] (8-bit)
        uint8_t type = ram[0x2000 + x]; 
        if ((type & 0x1F) == 0x13) {    // and #$1f, cmp #$13, bne @ffd7
            // This is Fusoya
            uint8_t status_lo = ram[0x2003 + x];
            if ((status_lo & 0xC0) != 0) { // and #$c0, bne @ffd0
                ram[0x357C] = 0xFF;      // disable regen
                return;                  // bra @ffe5
            }
            uint8_t status_hi = ram[0x2004 + x];
            if ((status_hi & 0x3C) == 0) { // and #$3c, beq @ffe5
                // If NOT (paralyze, sleep, charm, berserk), then it's a valid state
                // The ASM logic: if beq @ffe5, then it continues to @ffd0
                // Wait, beq @ffe5 is the EXIT. So if (status & 0x3C) == 0, it EXITS.
                // If (status & 0x3C) != 0, it falls through to @ffd0.
                // Let's re-read:
                // lda $2004,x
                // and #$3c
                // beq @ffe5 (if zero, exit)
                // @ffd0: lda #$ff / sta $357c / bra @ffe5
                // Conclusion: if (status & 0x3C) != 0, disable regen and exit.
                ram[0x357C] = 0xFF;
                return;
            }
            // Fall through to @ffd0 if the beq was not taken? 
            // No, the logic is:
            // if (status_hi & 0x3C) != 0 { ram[0x357C] = 0xFF; return; }
            // But @ffd0 is AFTER the beq @ffe5. 
            // If (status_hi & 0x3C) != 0 -> falls through to @ffd0 -> disable regen -> exit.
            // If (status_hi & 0x3C) == 0 -> branches to @ffe5 -> exit.
            // This means if status is 0, regen remains enabled.
        } else {
            // Not Fusoya: skip this character block
            x += 0x80; // longa, txa, clc, adc #$0080, tax, shorta0
        }
        y--; // dey
    }
}

// PITFALLS: 1 (DB=$FF for char data), 6 (A 8-bit vs 16-bit transition for X increment),
//           8 (Inherited mf=true)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2000=1, 0x2003=1, 0x2004=1 (scaled by X)
//   output_ram:  0x357C=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFF
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckRegen ($FF:B4)