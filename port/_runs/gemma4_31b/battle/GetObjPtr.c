// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B1 (Code), DP=0
// Logic: Calculates the memory pointer for a battle object.
// If bit 7 of A is set, it's a monster; otherwise, it's a player/ally.
// The index (A & 0x7F) is multiplied by 0x80 via Mult8 to get the offset.
static void GetObjPtr_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    // pha / and #$7f / sta $df
    ram[0xDF] = a & 0x7F;
    // lda #$80 / sta $e1
    ram[0xE1] = 0x80;

    // jsr Mult8: result in ram[0xE3] (lo), ram[0xE4] (hi)
    Mult8_emu(snes);

    // pla / bmi @b180: check if bit 7 of original A was set
    if (a & 0x80) {
        // @b180: Monster path
        uint8_t lo = ram[0xE3];
        uint8_t hi = ram[0xE4];

        // clc / lda $e3 / adc #$80 / sta $80
        uint16_t res_lo = (uint16_t)lo + 0x80;
        ram[0x80] = (uint8_t)res_lo; // Pitfall 7: truncate to 8-bit
        
        // lda $e4 / adc #$22 / sta $81
        uint8_t carry = (res_lo > 0xFF) ? 1 : 0;
        ram[0x81] = (uint8_t)(hi + 0x22 + carry); // Pitfall 7
    } else {
        // Non-monster path
        uint8_t lo = ram[0xE3];
        uint8_t hi = ram[0xE4];

        // clc / lda $e3 / adc #$00 / sta $80
        ram[0x80] = lo; 
        
        // lda $e4 / adc #$20 / sta $81
        // Carry is 0 because we added 0x00 to lo
        ram[0x81] = (uint8_t)(hi + 0x20); // Pitfall 7
    }
}

// PITFALLS: 7 (ADC chain results truncated to 8-bit to match 65816 A=8-bit behavior)
// HELPERS: Mult8_emu(snes) — delegates Mult8 @ $83E0
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x80=1, 0x81=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB1
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetObjPtr ($B1:63)