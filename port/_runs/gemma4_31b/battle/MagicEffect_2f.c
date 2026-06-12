// Logic:
// Decrements the value at WRAM $3881.
static void MagicEffect_2f_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x3881]--;
}

// PITFALLS: 7 (Arithmetic truncation: dec is performed on 8-bit memory,
// wrapping from 0x00 to 0xFF automatically via uint8_t underflow)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3881=1
//   output_ram:  0x3881=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_2f ($DD:DD)