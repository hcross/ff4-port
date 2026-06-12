// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine calculates a pointer to an object based on an index passed in A.
// It determines if the object is a monster (bit 7 set) and calculates the offset.
// In: cpu->a = object_index (8-bit)
// Out: ram[0x80-0x81] = object_pointer (16-bit LE)
static void GetObjPtr_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    // pha / and #$7f / sta $df
    ram[0xDF] = a & 0x7F;
    // lda #$80 / sta $e1
    ram[0xE1] = 0x80;

    // jsr Mult8: calculates ram[0xDF] * ram[0xE1]
    // Result is typically stored in ram[0xE3-0xE4]
    mult8_emu(snes);

    // pla / bmi @b180 (checks if original A had bit 7 set)
    if (a & 0x80) {
        // @b180: monster path
        uint8_t lo = ram[0xE3];
        uint8_t hi = ram[0xE4];
        
        // clc / lda $e3 / adc #$80 / sta $80
        ram[0x80] = (uint8_t)(lo + 0x80);
        // lda $e4 / adc #$22 / sta $81
        // Carry from previous addition is simulated by checking overflow
        uint16_t res = (uint16_t)lo + 0x80;
        uint8_t carry = (res > 0xFF) ? 1 : 0;
        ram[0x81] = (uint8_t)(hi + 0x22 + carry);
    } else {
        // Non-monster path
        uint8_t lo = ram[0xE3];
        uint8_t hi = ram[0xE4];
        
        // clc / lda $e3 / adc #$00 / sta $80
        ram[0x80] = lo; 
        // lda $e4 / adc #$20 / sta $81
        // No carry since we added 0 to lo
        ram[0x81] = (uint8_t)(hi + 0x20);
    }
}

// PITFALLS: 7 (Arithmetic truncation to 8-bit for ADC chain)
// HELPERS: mult8_emu(snes) — delegates Mult8 @ $83E0
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram: 0x80=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetObjPtr ($B1:63)