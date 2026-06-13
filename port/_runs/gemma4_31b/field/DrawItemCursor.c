#include "snes/snes.h"

// Entry mode: A 8-bit (mf=true), X 16-bit (xf=false), DB=0x00, DP=0x00
// Purpose: Draws the item selection cursor in VRAM. 
// Checks window height, clears previous cursor if active, calculates VRAM offset 
// from scroll/column, and writes the cursor tile.
static void DrawItemCursor_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0xDA] != 0x08) { // lda $da / cmp #$08 / beq @b121
        return;
    }

    // Hardware VMA registers are accessed via snes->vma_xxx in this port
    snes->vma_inc = 0x80; // sta hVMAINC

    if (ram[0xE7] != 0) { // lda $e7 / beq @b13b
        snes->vma_addr = ram[0x8D]; // ldx $8d / stx hVMADDL
        ram[0xE7] = 0;              // stz $e7
        snes->vma_data = 0x20FF;    // lda #$ff / sta hVMDATAL / lda #$20 / sta hVMDATAH
    }

    // VRAM address calculation
    uint16_t sum = (uint16_t)ram[0xBA] + ram[0x8C]; // lda $ba / clc / adc $8c
    uint8_t low = (uint8_t)sum;                    // sta $4b
    uint8_t high = (uint8_t)(sum >> 8);             // stz $4a (sum < 256 here, but as a 16-bit result)

    // Shift right by 2 (lsr $4b / ror $4a x2)
    uint16_t val = (uint16_t)(low | (high << 8));
    val >>= 2;
    ram[0x4B] = (uint8_t)(val & 0xFF);
    ram[0x4A] = (uint8_t)(val >> 8);

    uint8_t a = ram[0x8B];
    if (a != 0) {
        a = 0x0D; // lda #$0d
    }

    // Row/Col offset: a = a + 0x23 + ram[0x4A]
    // Pitfall 7: wrap in uint8_t to truncate carry
    uint8_t res_a = (uint8_t)((uint8_t)(a + 0x23) + ram[0x4A]); 
    ram[0x4A] = res_a;

    // Tile offset: ram[0x4B] = (ram[0x4B] & 0x03) + 0x2C
    uint8_t res_b = (uint8_t)((ram[0x4B] & 0x03) + 0x2C);
    ram[0x4B] = res_b;

    uint16_t final_addr = (uint16_t)(ram[0x4B] | (ram[0x4A] << 8));
    ram[0x8D] = (uint8_t)(final_addr & 0xFF); // ldx $4a / stx $8d (asm is actually ldx $4a, which is 8-bit)
    
    snes->vma_addr = final_addr; // stx hVMADDL
    snes->vma_data = 0x2014;     // lda #$14 / sta hVMDATAL / lda #$20 / sta hVMDATAH
}

// PITFALLS: 7 (8-bit arithmetic truncation for ADC chain), 1 (DP=0, DB=0)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xDA=1, 0xE7=1, 0x8D=1, 0xBA=1, 0x8C=1, 0x8B=1
//   output_ram:  0x8D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DrawItemCursor ($B1:1A)