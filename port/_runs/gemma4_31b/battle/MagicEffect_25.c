// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DD (or $7E for WRAM), DP=0
// Logic:
//   - Sets a value (0x05) in ram[0x38E6]
//   - Reads a 16-bit value from ram[0x2687]
//   - Caps that value at 9999 (Bugfix Rev1)
//   - Writes the result to ram[0xA4] (16-bit)
//   - Sets ram[0x2683] to 0x80
static void MagicEffect_25_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x38E6] = 0x05;

    // longa: A becomes 16-bit
    uint16_t val = read16(ram, 0x2687);

    // .if BUGFIX_REV1 logic
    if (val > 9999) { // cmp #9999 / bcc @dd27 (inversion: if not lower, then cap)
        val = 9999;
    }

    // sta $a4 (A is 16-bit)
    write16(ram, 0xA4, val);

    // shorta0: A = D (usually 0), then A becomes 8-bit
    ram[0x2683] = 0x80;
}

// PITFALLS: 6 (Mode A transition: 8-bit -> 16-bit -> 8-bit). 
// The routine explicitly switches A size via longa and shorta0.
// PITFALL 1: Routine accesses WRAM at $38E6 and $A4, requiring DB=$7E 
// mapping context even if PC is in $DD.

// HELPERS: read16/write16 - little-endian 16-bit accessors.

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2687=2
//   output_ram: 0x38E6=1, 0x00A4=2, 0x2683=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_25 ($DD:1D)