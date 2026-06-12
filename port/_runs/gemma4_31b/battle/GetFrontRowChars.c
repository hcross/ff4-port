// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
// Iterates through 5 characters. Checks presence and status (alive, 
// not stone, not magnetized, not jumping, not hiding, and in front row).
// If a character meets all criteria, the corresponding bit in ram[$ab] is set.
static void GetFrontRowChars_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // clr_axy / stx $ab
    // DP is 0, so A=X=Y=0
    uint16_t x = 0;
    uint16_t y = 0;
    ram[0xAB] = 0;

    while (x < 5) { // cpx #5 / bne @b983
        // lda $3540,x (X is 16-bit, but A is 8-bit)
        uint8_t presence = ram[0x3540 + x];
        if (presence != 0) goto skip_char; // bne @b9a7

        // lda $2003,y (Y is 16-bit, A is 8-bit)
        uint8_t status1 = ram[0x2003 + y];
        if ((status1 & 0xC0) != 0) goto skip_char; // bne @b9a7

        // lda $2005,y
        uint8_t status2 = ram[0x2005 + y];
        if ((status2 & 0x82) != 0) goto skip_char; // bne @b9a7

        // lda $2006,y
        uint8_t status3 = ram[0x2006 + y];
        if ((int8_t)status3 < 0) goto skip_char; // bmi @b9a7

        // lda $2001,y
        uint8_t status4 = ram[0x2001 + y];
        if ((int8_t)status4 < 0) goto skip_char; // bmi @b9a7

        // lda $ab / jsr SetBit / sta $ab
        uint8_t bitmask = ram[0xAB];
        snes->cpu->a = bitmask; 
        set_bit_emu(snes); // jsr SetBit (delegated)
        ram[0xAB] = (uint8_t)snes->cpu->a;

    skip_char:
        // longa / tya / clc / adc #$0080 / tay / shorta0
        // This calculates the offset for the next character's data block
        // Y is 16-bit. result = Y + 0x80.
        y = y + 0x80;

        // inx
        x++;
    }
}

// PITFALLS: 6 (Mixed A-mode: routine switches between 8-bit for flags 
// and 16-bit for Y-offset math), 8 (Inherited mode mf=true)
// HELPERS: set_bit_emu(snes) — delegates SetBit @ $00:855F
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3540=1, 0x2003=1, 0x2005=1, 0x2006=1, 0x2001=1
//   output_ram:  0xAB=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetFrontRowChars ($B9:7E)