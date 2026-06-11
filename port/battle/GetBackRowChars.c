// Scans the first 5 characters in battle (indexed by X) and checks
// several status flags to determine if they are back-row eligible.
// For each eligible character, sets the corresponding bit in $ab.
// Uses Y to index into $2000-style character records (128 bytes apart).
static void GetBackRowChars_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t x = 0;
    uint8_t y = 0;
    ram[0xAB] = 0;

    do {
        if (ram[0x3540 + x] != 0) goto next;         // bne @b9f2
        if (ram[0x2003 + y] & 0xC0) goto next;       // and #$c0 / bne @b9f2
        if (ram[0x2005 + y] & 0x82) goto next;       // and #$82 / bne @b9f2
        if (ram[0x2006 + y] & 0x80) goto next;       // bmi @b9f2
        if ((ram[0x2001 + y] & 0x80) == 0) goto next; // bpl @b9f2

        // Character is eligible — set bit X in $ab
        snes->cpu->a = ram[0xAB];
        snes->cpu->x = x;
        setbit_emu(snes);                            // jsr SetBit
        ram[0xAB] = snes->cpu->a;

    next:
        y += 0x80;                                   // longa / tya / clc / adc #$0080 / tay
        x++;                                         // inx
    } while (x != 5);                                // cpx #5 / bne @b9ce
}

// PITFALLS: 1 (DB=$7E), 6 (A starts 8-bit), 8 (A/X mode inherited as 8-bit/16-bit)
// HELPERS: setbit_emu(snes) — delegates SetBit @ $03:855F
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x3540=1, 0x2001=1, 0x2003=1, 0x2005=1, 0x2006=1
//   output_ram:  0x00AB=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetBackRowChars ($B9:C9)