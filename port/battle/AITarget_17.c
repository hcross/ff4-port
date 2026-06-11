// AITarget_17 - writes $FF to a dynamically-indexed target table entry
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
static void AITarget_17_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    uint16_t x = read16(ram, 0xA6);  // ldx $a6 (X 16-bit)
    ram[0x2053 + x] = 0xFF;          // lda #$ff / sta $2053,x
}

// PITFALLS: 1 (DB=$7E required for absolute addressing),
//           8 (mode inherited: mf=true, xf=false per battle convention)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x00A6=2
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::AITarget_17 ($B9:18)