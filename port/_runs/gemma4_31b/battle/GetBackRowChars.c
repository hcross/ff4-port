// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$B9 (Wait: DB is set by caller), DP=0
// Logic:
// Iterates through 5 characters (X=0..4). For each character, it checks specific 
// flag bits in a data block (indexed by Y). If the conditions are met, it uses 
// SetBit to update a bitmask in RAM $AB. The data block offset Y increments by 0x80 
// each iteration.
static void GetBackRowChars_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // clr_axy / stx $ab
    // In battle context, TDC clears registers if DP=0
    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    ram[0xAB] = 0;

    for (uint16_t x = 0; x < 5; x++) {
        // lda $3540,x
        uint8_t char_val = ram[0x3540 + x];
        if (char_val != 0) {               // bne @b9f2 (True if A != 0)
            goto next_iter;
        }

        // lda $2003,y / and #$c0 / bne @b9f2
        if ((ram[0x2003 + cpu->y] & 0xC0) != 0) goto next_iter;

        // lda $2005,y / and #$82 / bne @b9f2
        if ((ram[0x2005 + cpu->y] & 0x82) != 0) goto next_iter;

        // lda $2006,y / bmi @b9f2
        // bmi branches if bit 7 is 1 (negative)
        if ((int8_t)ram[0x2006 + cpu->y] < 0) goto next_iter;

        // lda $2001,y / bpl @b9f2
        // bpl branches if bit 7 is 0 (positive)
        if ((int8_t)ram[0x2001 + cpu->y] >= 0) goto next_iter;

        // lda $ab / jsr SetBit / sta $ab
        cpu->a = ram[0xAB];
        SetBit_emu(snes);                 // Result is returned in A
        ram[0xAB] = (uint8_t)cpu->a;

    next_iter:
        // @b9f2: longa / tya / clc / adc #$0080 / tay / shorta0
        // Y is incremented by 0x80
        cpu->y = (uint16_t)(cpu->y + 0x80);
        
        // inx / cpx #5 / bne @b9ce
        // Handled by for-loop condition (x < 5)
    }
}

// PITFALLS: 6 (Mode A switching for Y increment), 8 (Inherited mf=true)
// HELPERS: SetBit_emu(snes) — delegates SetBit @ $855F
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3540=1, 0x2001=1, 0x2003=1, 0x2005=1, 0x2006=1
//   output_ram:  0xAB=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetBackRowChars ($B9:C9)