#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (assuming field default), DP=0
// Purpose: Updates the lava palette in WRAM if the player is underground.
// It selects a palette base from LavaPalTbl based on bits 4-6 of $7A, 
// then copies 16 words (32 bytes) from LavaAnimPal to $0CFF.
static void UpdateLavaPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $1700 / cmp #1 / bne @c4fc
    if (ram[0x1700] != 1) {
        return; // Only execute for underground (1)
    }

    // lda $7a / and #$70 / lsr4 / tax
    // Extraction of bits 4-6 shifted right by 4
    uint8_t index = (ram[0x7A] & 0x70) >> 4;

    // lda f:LavaPalTbl,x / tax
    // f:LavaPalTbl is in ROM. We assume an external helper or direct ROM access.
    // Based on the pattern, we read the table and store the resulting value in X.
    uint16_t pal_base = read16(snes->rom, LAVA_PAL_TBL + (index * 2));

    // ldy #0 / longa
    // Copy 16 words from LavaAnimPal + pal_base to $0CFF
    for (uint16_t y = 0; y < 16; y++) {
        // lda f:LavaAnimPal,x / sta $0cff,y
        // Note: y is incremented by 2 (iny2), so we iterate 16 times over words
        uint16_t val = read16(snes->rom, LAVA_ANIM_PAL + pal_base);
        write16(ram, 0x0CFF + (y * 2), val);
        
        // inx2 (X is the pointer into LavaAnimPal)
        pal_base += 2;
    }
    
    // lda #0 / shorta
    // Restore A to 8-bit mode as per asm's shorta
}

// PITFALLS: 6 (Mode A switches from 8-bit to 16-bit for the copy loop and back),
//           7 (LSR4 used for 8-bit truncation/shifting).
// HELPERS: read16, write16.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x1700=1, 0x7A=1
//   output_ram:  0x0CFF=32 (16 words)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::UpdateLavaPal ($C4:CD)