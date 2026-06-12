// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM:
//   in : ram[$273B] base value, ram[$289D] modifier
//   out: ram[$273B] updated value (clamped to [12, 32])
static void MagicEffect_0f_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t base = ram[0x273B];
    uint8_t mod = ram[0x289D];
    uint16_t sum = (uint16_t)(base + mod);  // clc / adc (8-bit operands)
    uint8_t result;
    if (sum >= 0x20) {                      // cmp #$20 / bcs @d89a
        result = 0x20;                      // max 32
    } else if (sum >= 0x0C) {               // cmp #$0c / bcs @d89c
        result = (uint8_t)sum;              // keep sum
    } else {
        result = 0x0C;                      // max 12
    }
    ram[0x273B] = result;                   // sta $273b
}

// PITFALLS: 7 (arithmetic truncation: sum must be 8-bit truncated to match
// carry-out behavior in 8-bit mode, but we simulate it with uint16_t then cast)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x273B=1, 0x289D=1
//   output_ram:  0x273B=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_0f ($D8:87)