#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C3, DP=0
// Purpose: Determines the source memory offset and length for sprite graphics 
// based on the input index in A, then configures the transfer parameters for Tfr3bppGfx.
static void LoadSpriteGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a_in = (uint8_t)snes->cpu->a;
    uint16_t scale = 0;
    uint16_t x_val = 0;

    if (a_in < 0x11) {                      // cmp #$11 / bcs @c398
        scale = (uint16_t)(a_in << 3);      // asl3
        x_val = 0;                          // ldx #0
    } else if (a_in < 0x30) {              // cmp #$30 / bcs @c3a7
        scale = (uint16_t)((a_in - 0x11) << 2); // sbc #$11 / asl2
        x_val = 0x3300;                     // ldx #$3300
    } else if (a_in < 0x46) {              // cmp #$46 / bcs @c3b5
        scale = (uint16_t)((a_in - 0x30) << 1); // sbc #$30 / asl
        x_val = 0x6180;                     // ldx #$6180
    } else {                            // sec / sbc #$46
        scale = (uint16_t)(a_in - 0x46);
        x_val = 0x7200;                     // ldx #$7200
    }

    // --- 16-bit math block (longa) ---
    // xba / lsr2 / sta $4a / lsr / adc $4a / sta $4a
    // The ASM performs a 16-bit shift then stores/adds to 8-bit RAM $4a.
    uint16_t val = scale; 
    uint16_t shifted2 = val >> 2;           // lsr2
    uint8_t scratch = (uint8_t)(shifted2 & 0xFF); // sta $4a (low byte)
    
    uint16_t shifted1 = shifted2 >> 1;      // lsr
    // adc $4a (A is shifted1, adding scratch)
    uint8_t result_scratch = (uint8_t)((shifted1 + scratch) & 0xFF); 
    ram[0x4A] = result_scratch;             // sta $4a

    // txa / clc / adc $4a / clc / adc #$8000 / sta $4a
    // A = x_val, add the scratch result, add 0x8000, store low byte
    uint32_t final_addr = (uint32_t)x_val + result_scratch + 0x8000;
    ram[0x4A] = (uint8_t)(final_addr & 0xFF); // sta $4a

    // --- 8-bit mode (shorta) ---
    uint8_t ae = ram[0xAE];
    ram[0x4D] = (uint8_t)(((uint16_t)ae << 1) + 0x42); // lda $ae / asl / adc #$42 / sta $4d
    ram[0x4C] = 0;                          // stz $4c
    write16(ram, 0x4E, 0x0200);             // ldx #$0200 / stx $4e
    
    // MapSpriteGfx bank byte (0x03 per disassembly/map)
    ram[0x49] = 0x03; 

    Tfr3bppGfx_emu(snes);                   // jsl Tfr3bppGfx
}

// PITFALLS: 1 (DB=$C3), 6 (Mode A transition long/short), 7 (8-bit truncation 
// during $4a scratchpad arithmetic).
// HELPERS: Tfr3bppGfx_emu(snes) — delegates Tfr3bppGfx @ $860F
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0xAE=1
//   output_ram:  0x4A=1, 0x4C=1, 0x4D=1, 0x4E=2, 0x49=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0xC3
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadSpriteGfx ($C3:8B)