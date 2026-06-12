// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Logic:
// Iterates through 5 characters (X=0..4).
// Checks if character at index X is "back row" by verifying a sequence of flags
// in RAM across different base addresses. If all conditions meet, 
// it calls SetBit to mark the character in a bitmask stored at $AB.
static void GetBackRowChars_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // clr_axy / stx $ab
    // Assuming DP=0, clr_axy clears A, X, Y via TDC.
    uint16_t mask = 0; 
    ram[0xAB] = 0;

    for (uint16_t x = 0; x < 5; x++) {
        // lda $3540,x
        uint8_t char_val = ram[0x3540 + x];
        if (char_val == 0) {
            goto next_iter; // bne @b9f2
        }

        // The routine uses Y as an index for subsequent checks.
        // Based on the ASM: lda $2003,y ... lda $2001,y
        // However, Y is only modified at @b9f2 via: longa / tya / adc #$0080 / tay
        // On first iteration, Y=0.
        uint16_t y = (uint16_t)snes->cpu->y;

        // lda $2003,y / and #$c0 / bne @b9f2
        if ((ram[0x2003 + y] & 0xC0) != 0) goto next_iter;

        // lda $2005,y / and #$82 / bne @b9f2
        if ((ram[0x2005 + y] & 0x82) != 0) goto next_iter;

        // lda $2006,y / bmi @b9f2
        // bmi: branch if minus (bit 7 is 1). If ram[0x2006+y] >= 0x80, it branches.
        if ((int8_t)ram[0x2006 + y] < 0) goto next_iter;

        // lda $2001,y / bpl @b9f2
        // bpl: branch if plus (bit 7 is 0). If ram[0x2001+y] < 0x80, it branches.
        if ((int8_t)ram[0x2001 + y] >= 0) goto next_iter;

        // lda $ab / jsr SetBit / sta $ab
        uint8_t current_mask = ram[0xAB];
        snes->cpu->a = current_mask;
        // SetBit is delegated; result is returned in A
        uint16_t result = set_bit_emu(snes); 
        ram[0xAB] = (uint8_t)result;

    next_iter:
        // @b9f2: longa / tya / clc / adc #$0080 / tay / shorta0
        // This increments Y by 0x80 for the next character's data block.
        uint16_t next_y = (uint16_t)snes->cpu->y + 0x80;
        snes->cpu->y = next_y;
        
        // inx / cpx #5 / bne @b9ce
        // (Handled by for loop)
    }
}

// PITFALLS: 1 (DB=$7E), 6 (Mode A switching between 8-bit and 16-bit for Y increment), 
// 8 (Inherited mf=true for battle module).
// HELPERS: set_bit_emu(snes) — delegates SetBit @ $855F
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=0
//   inputs_ram:  0x3540=1, 0x2001=1, 0x2003=1, 0x2005=1, 0x2006=1
//   output_ram:  0xAB=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetBackRowChars ($B9:C9)