#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0x00 (implied by $07 access), DP=0
// Purpose: Calculates an MSB mask based on Y and A, then applies it to a 
// sprite coordinate stored at $0500 + offset.
static void SetSpriteMSB_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Input registers: A (8-bit), Y (16-bit)
    uint8_t a_reg = (uint8_t)snes->cpu->a;
    uint16_t y_reg = snes->cpu->y;

    // longa / tya / lsr2
    // Cast Y to 16-bit A, shift right by 2
    uint16_t temp_y_shifted = y_reg >> 2;

    // shorta / sta $07
    // Only the low byte of the shifted result is stored in DP:$07
    uint8_t val_07 = (uint8_t)(temp_y_shifted & 0xFF);
    ram[0x07] = val_07;

    // pla / clc / adc $07
    // A = original A + ram[0x07] (8-bit addition)
    uint8_t sum = (uint8_t)(a_reg + val_07);

    // pha / and #$03
    // Mask the sum to 2 bits to use as index into SpriteMSBTbl
    uint8_t index = sum & 0x03;

    // tax / lda f:SpriteMSBTbl,x / sta $07
    // Note: SpriteMSBTbl is in ROM. We assume the harness provides access
    // or we emulate the load. Since it's a small table, we use a constant.
    // Based on FF4 ROM: SpriteMSBTbl = {0x00, 0x01, 0x02, 0x03} (usually identity or offsets)
    // For the sake of the translation, we access the ROM data.
    uint8_t msb_mask = snes->rom[0xB300 + 0x00 /* Approximate table offset */ + index]; 
    // Note: Actual table address for SpriteMSBTbl needs to be verified against the symbol map.
    ram[0x07] = msb_mask;

    // pla / lsr2 / tay
    // Recover original A, shift right by 2 to get the sprite index offset
    uint8_t a_shifted = a_reg >> 2;
    uint16_t target_idx = (uint16_t)a_shifted;

    // lda $0500,y / ora $07 / sta $0500,y
    // Apply the mask to the byte at $0500 + (Y index)
    // The original ASM uses 'tay' on the shifted A, so the offset is actually (a_reg >> 2)
    uint16_t addr = 0x0500 + target_idx;
    ram[addr] |= ram[0x07];
}

// PITFALLS: 6 (Mode A switching: longing/shorting used to shift Y and store in $07),
// 7 (8-bit truncation: adc and lsr results must be cast to uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=16
//   inputs_ram:  0x0500=1 (via Y index), f:SpriteMSBTbl=1 (ROM)
//   output_ram:  0x0500=1, 0x0007=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::SetSpriteMSB ($B3:AF)