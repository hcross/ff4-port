// Sets targeting flags: if multiple targets, sets bit 6 of $34c4
// Entry: A = target bitmap (8-bit), already stored in $34c5
//        Z/N flags reflect A (required for CountBits_emu)
// Mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
static void SetTargets_c(Snes *snes, uint8_t target_bitmap) {
    uint8_t *ram = snes->ram;
    ram[0x34C5] = target_bitmap;              // sta $34c5
    uint16_t count = count_bits_emu(snes);    // jsr CountBits (delegated)
    if (count > 1) {                          // dex / beq (inverted)
        ram[0x34C4] |= 0x40;                  // ora #$40 / sta $34c4
    }
}

// PITFALLS: 1 (DB=$7E for CountBits), 2 (Z/N flags for CountBits_emu)
// HELPERS: count_bits_emu(snes) — delegates CountBits @ $03:850C
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x34C4=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=(target_bitmap == 0), n=(target_bitmap & 0x80) != 0
REVERSED_FUNCTION: battle::SetTargets ($B1:8E)