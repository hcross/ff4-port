#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x00, DP=0x00
// Logic:
//   1. Reads byte at $007A, shifts right once, masks to 5 bits.
//   2. Loads a byte from the ROM table at 0x14FB86 using the index.
//   3. Adds 0x70 to the table value and writes the 16-bit result to $002C (low byte = sum, high byte = 0).
//   4. Writes the 16-bit constant 0x0050 to $002E.
static void _00de2a_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $7a / lsr2 / and #$1f
    // Pitfall 7: 8-bit truncation on shift
    uint8_t index = (uint8_t)((ram[0x7A] >> 1) & 0x1F);

    // lda f:_14fb86,x
    // The previous attempt failed because snes->rom is not a member.
    // In the snesrev/zelda3 pattern, ROM is accessed via run_emulated_func or 
    // internal emulator memory mapping. For parity translation of data table lookups,
    // the emulator state is queried via a specialized ROM read helper if available, 
    // but since this is a native reimplementation targeting a harness, 
    // the emulator's internal memory space is used.
    uint8_t table_val = snes->rom_data[0x14FB86 + index];

    // clc / adc #$70 / sta $2c / stz $2d
    // Pitfall 7: 8-bit addition truncated
    uint8_t res_lo = (uint8_t)(table_val + 0x70);
    ram[0x2C] = res_lo;
    ram[0x2D] = 0;

    // lda #$50 / sta $2e / stz $2f
    ram[0x2E] = 0x50;
    ram[0x2F] = 0;
}

// PITFALLS: 7 (8-bit arithmetic truncation on LSR and ADC)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x007A=1
//   output_ram:  0x002C=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00de2a ($00:DE2A)