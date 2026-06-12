// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine initializes character row settings for specific memory 
// addresses (likely VRAM or registers mapped in WRAM) based on the 
// value at $16A8. It essentially toggles the 7th bit (0x80) 
// of five target addresses.
static void InitCharRows_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0x16A8] == 0) { // lda $16a8 / bne @95fc (not taken if 0)
        ram[0x2001] &= 0x7F; // and #$7f
        ram[0x2081] &= 0x7F; // and #$7f
        ram[0x2101] &= 0x7F; // and #$7f
        ram[0x2181] |= 0x80; // ora #$80
        ram[0x2201] |= 0x80; // ora #$80
    } else {                // @95fc
        ram[0x2001] |= 0x80; // ora #$80
        ram[0x2081] |= 0x80; // ora #$80
        ram[0x2101] |= 0x80; // ora #$80
        ram[0x2181] &= 0x7F; // and #$7f
        ram[0x2201] &= 0x7F; // and #$7f
    }
}

// PITFALLS: 1 (DB=$7E used for absolute addressing)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x16A8=1
//   output_ram:  0x2001=1, 0x2081=1, 0x2101=1, 0x2181=1, 0x2201=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::InitCharRows ($95:CE)