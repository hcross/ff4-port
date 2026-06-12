// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   The routine calculates a pointer/value by adding a 16-bit constant from 
//   WRAM [$3530-$3531] to the current value in the accumulator (A), 
//   carrying over the overflow to the high byte.
//   The result is stored in WRAM [$3598-$3599].
static void GetTimerPtr_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t a = snes->cpu->a; // Mode A 8-bit: use low byte

    // clc / adc $3530
    uint16_t low_res = (uint16_t)a + ram[0x3530];
    ram[0x3598] = (uint8_t)low_res; // sta $3598

    // lda $3531 / adc #0 / sta $3599
    // The carry from the previous addition is applied to the high byte
    uint16_t carry = low_res >> 8;
    uint16_t high_res = (uint16_t)ram[0x3531] + carry;
    ram[0x3599] = (uint8_t)high_res; // sta $3599
}

// PITFALLS: 7 (8-bit arithmetic truncation: result must be cast to uint8_t 
// to match 65816 behavior and carry generation)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x3530=1, 0x3531=1
//   output_ram: 0x3598=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetTimerPtr ($85:69)