// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Logic: 
//   Iterates through 5 potential characters (indexed by Y).
//   Checks if the character is present, alive, and not magnetized/jumping/hiding.
//   If a valid character is found, the routine returns immediately.
//   If no valid character is found after 5 iterations, ram[0xA9] is incremented.
static void FindValidChar_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // clr_axy (DP=0) and sty $a9
    uint16_t y = 0;
    ram[0xA9] = (uint8_t)y;

    while (y < 5) {
        // lda $3540,y
        uint8_t present = ram[0x3540 + y];
        if (present != 0) {
            goto next_obj; // bne @c425
        }

        // lda $2003,x
        // Note: X is 0 here due to clr_axy
        uint8_t status_dead = ram[0x2003 + snes->cpu->x];
        if ((status_dead & 0xC0) != 0) {
            goto next_obj; // bne @c425
        }

        // lda $2005,x
        uint8_t status_state = ram[0x2005 + snes->cpu->x];
        if ((status_state & 0x82) != 0) {
            goto next_obj; // bne @c425
        }

        // lda $2006,x / bpl @c430
        // bpl is true if bit 7 (sign) is 0. 
        // If (val & 0x80) == 0, it's not hiding, so we found a valid char and return.
        uint8_t status_hide = ram[0x2006 + snes->cpu->x];
        if ((status_hide & 0x80) == 0) {
            return; // bpl @c430
        }

    next_obj:
        next_obj_emu(snes); // jsr NextObj
        y++;                 // iny
        if (y == 5) {        // cpy #5 / bne @c40d
            ram[0xA9]++;     // inc $a9
            return;
        }
    }
}

// PITFALLS: 5 (clr_axy is a transfer from DP=0, effectively clearing A, X, Y)
// HELPERS: next_obj_emu(snes) — delegates NextObj @ 85BC
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3540=1, 0x2003=1, 0x2005=1, 0x2006=1
//   output_ram:  0x00A9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::FindValidChar ($C4:08)