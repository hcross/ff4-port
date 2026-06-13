#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0, DP=0 (inherited)
// Purpose: Copy a subset of the WhaleFlashPal palette into WRAM buffer $0EBB
// based on a seed value in ram[$7A].
static void LoadWhaleFlashPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $7a / and #$1e / tay
    uint8_t y = ram[0x7A] & 0x1E;

    // longa / ldx #0
    uint16_t x = 0;

    // The palette source WhaleFlashPal is at a fixed ROM address.
    // In the context of this port, we access ROM via snes->rom or a provided pointer.
    // Assuming WhaleFlashPal is defined in the project as a constant offset.
    const uint8_t *WhaleFlashPal = &snes->rom[0xWhaleFlashPal_Offset]; 

    do {
        // lda WhaleFlashPal,y / sta $0ebb,x
        ram[0x0EBB + x] = WhaleFlashPal[y];

        // tya / inc2 / and #$001f / tay
        y = (uint8_t)((y + 2) & 0x1F);

        // inx2
        x += 2;

        // cpx #16 / bne @cef9
    } while (x != 16);

    // lda #0 / shorta / rts
    snes->cpu->a = 0;
    snes->cpu->mf = true;
}

// PITFALLS: 6 (Mode A toggle: routine switches to 16-bit via longa then back to 8-bit via shorta)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7A=1
//   output_ram:  0x0EBB=16
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadWhaleFlashPal ($CE:EF)