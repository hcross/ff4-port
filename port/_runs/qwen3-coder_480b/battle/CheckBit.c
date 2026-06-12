// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x7E, DP=0
// Entry: cpu->a = bitmask, cpu->x = bit index
// Output: cpu->a = (bitmask & BitOrTbl[x])
static void CheckBit_c(Snes *snes, uint8_t bitmask, uint16_t bit_index) {
    uint8_t *ram = snes->ram;
    // Access BitOrTbl using 16-bit X index (xf=0), DB=0x7E
    uint8_t bit_flag = ram[0x7E0000 + 0x8580 + bit_index]; // f:BitOrTbl,x
    snes->cpu->a = bitmask & bit_flag;                      // and f:BitOrTbl,x
}

// PITFALLS: 1 (DB must be 0x7E to access BitOrTbl in bank 0x7E)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckBit ($85:64)