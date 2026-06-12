// Entry mode: A 16-bit on entry (starts with clr_a), X/Y 16-bit (longi assumed)
// All inputs/outputs in WRAM:
//   in : ram[$3540,X] = character presence (indexed by X=0..4)
//        ram[$2000,Y] = character data (indexed by Y=0,0x80,0x100,...)
//   out: ram[$AB] = bitfield of front-row characters (bit 0 = char 0, etc.)
static void GetFrontRowChars_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = 0;  // clr_ax → X = 0 (A also zero, but unused)
    uint16_t y = 0;  // clr_ay → Y = 0
    uint8_t ab = 0;  // stx $ab → X=0 stored to $ab

    for (int i = 0; i < 5; i++) {     // inx / cpx #5 / bne loop
        if (ram[0x3540 + x] != 0) goto next;  // bne @b9a7 (char not present)
        if ((ram[0x2003 + y] & 0xC0) != 0) goto next;  // dead/stone
        if ((ram[0x2005 + y] & 0x82) != 0) goto next;  // magnetized/jumping
        if ((ram[0x2006 + y] & 0x80) != 0) goto next;  // hiding
        if ((ram[0x2001 + y] & 0x80) != 0) goto next;  // back row

        // Character is present, alive, not hiding, in front row
        ab = set_bit_emu(snes, ab);  // jsr SetBit (delegated)

    next:
        y += 0x80;  // longa / tya / clc / adc #$0080 / tay
        x += 1;     // inx
    }

    ram[0xAB] = ab;  // final sta $ab
}

// PITFALLS: 6 (mode A starts 16-bit due to clr_a), 8 (inherited xf=0),
//           1 (DB=$7E assumed for all RAM access)
// HELPERS: set_bit_emu(snes, ab) — delegates SetBit @ $03:855F
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3540=1, 0x2001=1, 0x2003=1, 0x2005=1, 0x2006=1
//   output_ram:  0x00AB=1
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetFrontRowChars ($B9:7E)