// Writes 0xFF to ram[0x2053 + offset] where offset = ram[0xa6..a7]
// Entry mode: A 8-bit, X 16-bit, DB=$7E, DP=0
static void AITarget_17_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = read16(ram, 0xA6);      // ldx $a6
    ram[0x2053 + x] = 0xFF;              // lda #$ff / sta $2053,x
}

// PITFALLS: 1 (DB=$7E assumed), 8 (inherited modes: mf=true, xf=false)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x00A6=2
//   output_ram:  0x2053=1  # indexed by X from [0x00A6]
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
REVERSED_FUNCTION: battle::AITarget_17 ($03:B918)