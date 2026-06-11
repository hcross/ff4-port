// This function identifies front-row characters and builds a bitmask in $ab.
// It iterates over 5 characters, checking their presence, status, and row.
static void GetFrontRowChars_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    // Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
    // clr_axy → A = X = Y = 0 (via tdc/tax/tay)
    uint16_t x = 0;
    uint16_t y = 0;
    uint8_t result = 0; // $ab

    do {
        if (ram[0x3540 + x] != 0) goto next;         // bne @b9a7 (not present)
        if ((ram[0x2003 + y] & 0xC0) != 0) goto next; // dead/stone
        if ((ram[0x2005 + y] & 0x82) != 0) goto next; // magnetized/jumping
        if ((int8_t)ram[0x2006 + y] < 0) goto next;   // hiding (BMI)
        if ((int8_t)ram[0x2001 + y] < 0) goto next;   // back row (BMI)

        // Character is present, alive, not hiding, not magnetized/jumping, and in front row
        snes->cpu->a = result;
        snes->cpu->x = x;
        result = setbit_emu(snes); // jsr SetBit

    next:
        // longa / tya / clc / adc #$0080 / tay / shorta0
        y += 0x80;
        x++;
    } while (x < 5); // cpx #5 / bne @b983

    ram[0xAB] = result;
}

// PITFALLS: 6 (mode A starts 8-bit), 8 (X/Y are 16-bit by convention)
// HELPERS: setbit_emu(snes) — delegates SetBit @ $03:855F
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x2001=1, 0x2003=1, 0x2005=1, 0x2006=1, 0x3540=1
//   output_ram:  0x00AB=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetFrontRowChars ($B9:7E)