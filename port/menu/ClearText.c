#include "snes/snes.h"

// Logic:
// This routine clears two contiguous blocks of memory starting at (Y + 0x29) 
// and (Y + 0x29 + 0x40). The length of each block is determined by the 
// value in ram[0x1F] (with a conditional decrement if the Carry flag is clear).
// The memory is cleared by writing the value of ram[0x1F] into each byte.
static void ClearText_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // stz $1f
    ram[0x1F] = 0; 
    
    // Note: The ASM performs 'stz $1f' then 'bcs @fea4' / 'dec $1f'.
    // 'stz' always clears the Carry flag (C=0).
    // Therefore, 'bcs' is NEVER taken here. 'dec $1f' always executes.
    // ram[0x1F] becomes 0xFF.
    if (!cpu->c) { // bcs @fea4 (Carry is clear after stz)
        ram[0x1F]--; // dec $1f
    }

    // pha / sta $1d / sta $1e
    uint8_t count = ram[0x1F];
    ram[0x1D] = count;
    ram[0x1E] = count;

    // longa / tya / clc / adc $29 / tay / shorta
    // Y is 16-bit (longi convention for menu module).
    uint16_t y_reg = cpu->y;
    uint16_t ptr1 = y_reg + 0x29;
    
    // phy
    uint16_t y_saved = y_reg;

    // lda $1f / loop 1
    uint8_t fill_val = ram[0x1F];
    uint8_t loop1_count = ram[0x1D];
    for (uint16_t i = 0; i < loop1_count; i++) {
        ram[ptr1 + (i * 2)] = fill_val; // sta a:$0000,y / iny2
    }

    // ply
    y_reg = y_saved;

    // longa / tya / clc / adc #$0040 / tay / shorta
    uint16_t ptr2 = y_reg + 0x29 + 0x40;

    // lda $1f / loop 2
    uint8_t loop2_count = ram[0x1E];
    for (uint16_t i = 0; i < loop2_count; i++) {
        ram[ptr2 + (i * 2)] = fill_val; // sta a:$0000,y / iny2
    }

    // pla / rtl
}

// PITFALLS: 7 (8-bit arithmetic: dec $1f on 0 results in 0xFF), 
// 8 (Inherited mode: Y is 16-bit, A is 8-bit).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=16bits
//   inputs_ram:  none
//   output_ram:  none (writes to multiple areas based on Y)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto, c=auto
// CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: menu::ClearText ($FE:9E)