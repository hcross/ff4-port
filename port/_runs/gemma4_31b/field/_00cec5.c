#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (field), DP=0
// Purpose: Initializes two sets of parameters (likely coordinate-based or 
// animation state) in RAM starting at $0300 and $0304.
static void _00cec5_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // First block initialization
    ram[0x0300] = 0x68;
    
    // Second block initialization
    ram[0x0304] = 0x78;
    ram[0x0301] = 0x78;
    ram[0x0305] = 0x78;

    // Process input from $7A (Pitfall 7: 8-bit truncation)
    uint8_t a = ram[0x7A];
    a = (uint8_t)(a >> 1);      // lsr2 (LSR A)
    a = (uint8_t)(a & 0x02);    // and #$02
    
    // clc / adc #$e4
    uint8_t result = (uint8_t)(a + 0xE4); 
    ram[0x0302] = result;

    // eor #$02
    uint8_t eor_result = (uint8_t)(result ^ 0x02);
    ram[0x0306] = eor_result;

    // Final bytes
    ram[0x0303] = 0x37;
    ram[0x0307] = 0x37;
}

// PITFALLS: 7 (8-bit arithmetic truncation for LSR, AND, ADC, EOR)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x007A=1
//   output_ram:  0x0300=1, 0x0301=1, 0x0302=1, 0x0303=1, 0x0304=1, 0x0305=1, 0x0306=1, 0x0307=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00cec5 ($CE:C5)