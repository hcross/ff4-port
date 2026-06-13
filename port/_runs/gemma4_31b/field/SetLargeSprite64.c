#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E1 (implied by absolute $0500), DP=0
// This routine initializes a 16-byte block at $0500 (likely a sprite mask/pattern)
// by filling it with the constant value 0xAA (%10101010).
static void SetLargeSprite64_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // The asm uses absolute addressing $0500. 
    // In the SNES memory map, $0000-$07FF is typically part of the 
    // CGRAM or specific hardware registers depending on the bank.
    // Based on the context of "SetLargeSprite", this writes to the 
    // sprite data area.
    
    for (uint16_t x = 0; x < 16; x++) { // ldx #0 / inx / cpx #16 / bne
        ram[0x0500 + x] = 0xAA;        // lda #%10101010 / sta $0500,x
    }
}

// PITFALLS: None. Simple loop with constant value.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x0500=16 (writes 16 bytes starting at 0x0500)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE1
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::SetLargeSprite64 ($E1:CF)