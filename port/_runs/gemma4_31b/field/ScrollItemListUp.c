#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B0, DP=0
// This routine implements a timed delay for scrolling the item list.
// It iterates 8 times, waiting for a VBlank and decrementing a 
// global timer/counter ($BB) twice per iteration.
static void ScrollItemListUp_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x07] = 0x08; // lda #$08 / sta $07

    while (ram[0x07] != 0) { // loop @b090
        wait_vblank_long_emu(snes); // jsr WaitVblankLong
        
        ram[0xBB]--; // dec $bb
        ram[0xBB]--; // dec $bb
        
        ram[0x07]--; // dec $07
        // bne @b090 checks the result of dec $07
    }
}

// PITFALLS: 7 (Arithmetic truncation: ram[0x07] and ram[0xBB] are uint8_t, 
// mirroring the 8-bit DEC behavior where wrap-around occurs at 0x00->0xFF).
// HELPERS: wait_vblank_long_emu(snes) — delegates WaitVblankLong @ $00:912D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x07=1, 0xBB=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB0
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::ScrollItemListUp ($B0:8C)