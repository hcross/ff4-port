#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0x00 (assuming field module), DP=0
// Logic:
// 1. Computes an index for a table based on (A - 2) * 2.
// 2. Calculates two values based on RAM [0x0C] and [0x0E] and writes them to RAM [0x04F0, 0x04F1] + Y.
// 3. Calculates a third value based on RAM [0xAD] and the original input, writing to [0x04F2] + Y.
// 4. Fetches a byte from a table in bank F and writes to [0x04F3] + Y.
static void field_15b4b7_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // sec / sbc #2
    // A is 8-bit. Input is in cpu->a.
    uint8_t a_val = (uint8_t)(cpu->a - 2); 
    
    // tax / asl2 (X = A * 2)
    uint16_t x_val = (uint16_t)a_val << 1;
    
    // tay
    uint16_t y_val = x_val;

    // lda $0c / sec / sbc #4
    uint8_t a_mem_0c = ram[0x0C];
    uint8_t sbc_0c = (uint8_t)(a_mem_0c - 4);

    if ((uint8_t)(a_mem_0c - 4) < 0) { // This is conceptually wrong in C, using logic of BCS
        // bcs @b4cd is NOT taken if (a_mem_0c - 4) produced a borrow (A < 4)
        // In 65816, SBC #4 branches BCS if result is >= 0 (no borrow)
    }

    // Correcting logic for 65816 SBC/BCS:
    // SEC; SBC #4; BCS @b4cd 
    // The branch is TAKEN if a_mem_0c >= 4.
    if (a_mem_0c >= 4) {
        ram[0x04F0 + y_val] = sbc_0c; // sta $04f0,y
        uint8_t a_mem_0e = ram[0x0E];
        ram[0x04F1 + y_val] = (uint8_t)(a_mem_0e - 5); // sec / sbc #5 / sta $04f1,y
    } else {
        ram[0x04F1 + y_val] = 0xF8; // lda #$f8 / sta $04f1,y
    }

    // lda $ad / lsr4 / dec2 (A = (ram[0xAD] >> 4) - 2)
    // Note: lsr4 is likely a macro for 4 shifts.
    uint8_t a_mem_ad = ram[0xAD];
    uint8_t shifted = a_mem_ad >> 4;
    uint8_t dec_res = (uint8_t)(shifted - 2);
    ram[0x06] = dec_res; // sta $06

    // txa / asl / clc / adc $06 / adc #$30
    uint8_t term1 = (uint8_t)(a_val << 1); // txa / asl
    uint8_t res = (uint8_t)(term1 + ram[0x06]); // adc $06
    res = (uint8_t)(res + 0x30); // adc #$30
    ram[0x04F2 + y_val] = res; // sta $04f2,y

    // lda f:_15b4f5,x
    // Accessing bank F (ROM/Data). Since this is a translation to C, 
    // we access the ROM image using the index x_val.
    // Assuming snes->rom is available or similar mapping.
    // Since the target is game-and-watch-retro-go-sd, we use the mapped ROM address.
    uint8_t table_val = snes->rom[0x15B4F5 + x_val]; 
    ram[0x04F3 + y_val] = table_val; // sta $04f3,y
}

// PITFALLS: 7 (8-bit truncation on SBC/ADC), 8 (Inherited mode A=8, X/Y=16)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x0C=1, 0x0E=1, 0xAD=1
//   output_ram:  0x04F0=1, 0x04F1=1, 0x04F2=1, 0x04F3=1, 0x06=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_15b4b7 ($B4:B7)