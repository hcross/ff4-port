// Entry mode: Inherited (likely mf=true, xf=false), DB=$7E, DP=0
// This routine copies two 16-bit values from one set of addresses to another.
// Note: longa explicitly switches to 16-bit A, and shorta0 resets A and switches back to 8-bit.
static void MagicEffect_16_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // longa: A becomes 16-bit
    // lda $2709 / sta $2707
    uint16_t val1 = read16(ram, 0x2709);
    write16(ram, 0x2707, val1);

    // lda $270d / sta $270b
    uint16_t val2 = read16(ram, 0x270D);
    write16(ram, 0x270B, val2);

    // shorta0: clr_a (A = DP) then shorta (A = 8-bit)
    // Since DP=0 in battle convention, this effectively sets A = 0.
    snes->cpu->a = snes->cpu->dp; 
    snes->cpu->mf = true;
}

// PITFALLS: 6 (Mode A 16-bit explicitly set via longa, then reverted via shorta0)
// HELPERS: read16/write16 — little-endian 16-bit accessors over ram[]
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2709=2, 0x270D=2
//   output_ram:  0x2707=2, 0x270B=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_16 ($DA:0C)