// Scans the party (5 characters) and builds a bitmask ($ab) of back-row characters.
// A character is back-row if:
//   - $3540,x == 0        (not a mimic)
//   - $2003,y & $C0 == 0  (not hidden or dead)
//   - $2005,y & $82 == 0  (not stone or zombie)
//   - $2006,y >= 0        (not flee flag set)
//   - $2001,y < 0         (back row bit set)
//
// Entry mode: A 16-bit on entry (inherited), X/Y 16-bit (longi convention)
// DB = $7E, DP = 0
static void GetBackRowChars_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = 0;
    uint16_t y = 0;
    ram[0xAB] = 0;

    do {
        if (ram[0x3540 + x] != 0) goto next;              // bne @b9f2
        if ((ram[0x2003 + y] & 0xC0) != 0) goto next;     // and #$c0 / bne @b9f2
        if ((ram[0x2005 + y] & 0x82) != 0) goto next;     // and #$82 / bne @b9f2
        if ((int8_t)ram[0x2006 + y] < 0) goto next;       // bmi @b9f2
        if ((int8_t)ram[0x2001 + y] >= 0) goto next;      // bpl @b9f2

        // Character qualifies as back-row: set bit in $ab
        snes->cpu->a = ram[0xAB];
        SetBit_emu(snes);                                 // jsr SetBit
        ram[0xAB] = snes->cpu->a;

    next:
        y += 0x80;  // tya / clc / adc #$0080 / tay
        x++;        // inx
    } while (x < 5); // cpx #5 / bne @b9ce
}

// PITFALLS: 1 (DB=$7E), 6 (A starts 16-bit), 8 (X/Y 16-bit from longi)
// HELPERS: SetBit_emu(snes) — delegates SetBit @ $03:855F
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=16
//   inputs_ram:  0x2001=1, 0x2003=1, 0x2005=1, 0x2006=1, 0x3540=1
//   output_ram:  0x00AB=1
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetBackRowChars ($B9:C9)