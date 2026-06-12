// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   Adds a 16-bit value from WRAM [$3530-$3531] to the 8-bit accumulator.
//   The result is stored as a 16-bit word in WRAM [$3598-$3599].
static void GetTimerPtr_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    // clc / adc $3530
    uint16_t low_res = (uint16_t)a + ram[0x3530];
    ram[0x3598] = (uint8_t)low_res; // sta $3598

    // lda $3531 / adc #0 / sta $3599
    uint8_t carry = (uint8_t)(low_res >> 8); // Pitfall 7: capture carry from 8-bit add
    uint8_t high_res = (uint8_t)(ram[0x3531] + carry);
    ram[0x3599] = high_res; // sta $3599
}

// PITFALLS: 7 (8-bit arithmetic truncation used to derive carry for the high byte)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x3530=1, 0x3531=1
//   output_ram:  0x3598=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetTimerPtr ($85:69)