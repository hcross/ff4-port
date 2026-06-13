#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0xDF, DP=0
// Purpose: This routine iterates 4 times, reading coordinates from a table
// starting at _00df23, adding offsets from $f3/$f5, and calling two 
// delegated routines to likely update positions or handle drawing.
static void _00df53_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // stz $79
    ram[0x79] = 0;

    for (uint8_t i = 0; i < 4; i++) { // Loop from @df55, terminates when $79 == 4
        // lda $79 / asl / tay
        // Note: $79 is the loop counter. 
        // In 8-bit mode, i << 1 computes the offset for word-aligned table access.
        uint16_t offset = (uint16_t)(i << 1);
        
        // longa / lda _00df23,y
        // Address _00df23 is in the same bank (DF).
        uint16_t val_low = read16(ram, 0xDF23 + offset);
        
        // clc / adc $f3 / sta $0c
        // $f3 is DP-relative access (DP=0).
        uint16_t res_low = (uint16_t)(val_low + ram[0xF3]);
        write16(ram, 0x0C, res_low);

        // lda _00df2b,y / clc / adc $f5 / sta $0e
        uint16_t val_high = read16(ram, 0xDF2B + offset);
        uint16_t res_high = (uint16_t)(val_high + ram[0xF5]);
        write16(ram, 0x0E, res_high);

        // lda $79 / and #$00ff / asl4 / ora #$0080 / tay
        // i is 8-bit. asl4 is (i << 4).
        uint16_t y_val = (uint16_t)(((i & 0xFF) << 4) | 0x80);
        
        // lda #0 / shorta / jsr _00dfc4
        snes->cpu->a = 0;
        snes->cpu->mf = true; 
        snes->cpu->y = y_val; // Y was set by tay
        _00dfc4_emu(snes);

        // lda $0e / clc / adc #$10 / sta $0e
        uint16_t e_val = read16(ram, 0x0E);
        write16(ram, 0x0E, e_val + 0x10);

        // jsr _00e013
        _00e013_emu(snes);

        // inc $79
        ram[0x79]++;
    }
}

// PITFALLS: 7 (Arithmetic truncation in 8-bit mode for loop counter), 
// 8 (Inherited mf=true for battle/field routines), 1 (DB=0xDF).
// HELPERS: _00dfc4_emu(snes), _00e013_emu(snes), read16/write16.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xDF23=2, 0xDF2B=2, 0x00F3=1, 0x00F5=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDF
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00df53 ($DF:53)