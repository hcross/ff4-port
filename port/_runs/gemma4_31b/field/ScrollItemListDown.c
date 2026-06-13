#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x00, DP=0x00
// Purpose: Wait for 8 Vblanks and increment $BB twice per frame to 
//          create a scrolling delay effect for the item list.
static void ScrollItemListDown_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x07] = 0x08; // lda #$08 / sta $07
    
    do {
        wait_vblank_long_emu(snes); // jsr WaitVblankLong
        ram[0xBB]++;               // inc $bb
        ram[0xBB]++;               // inc $bb
        ram[0x07]--;               // dec $07
    } while (ram[0x07] != 0);      // bne @b0a0
}

// PITFALLS: None. Routine uses simple 8-bit decrement loop.
// HELPERS: wait_vblank_long_emu(snes) — delegates WaitVblankLong @ $B0:912D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xBB=1 (modified by inc), 0x07=1 (final value 0)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::ScrollItemListDown ($B0:9C)