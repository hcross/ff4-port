#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (SRAM/DP relative), DP=$1000
// Logic: Iterates through a buffer of 0x140 characters. For each character, 
// it clears the 6th bit (0x40) of the float status byte at $1004 + offset, 
// then advances to the next character via NextChar.
static void RemoveFloat_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    for (uint16_t x = 0; x != 0x0140; x++) {
        // lda $1004,x / and #$bf / sta $1004,x
        // DP is $1000, so $1004,x targets ram[0x1004 + x]
        ram[0x1004 + x] &= 0xBF;
        
        next_char_emu(snes); // jsr NextChar
    }
}

// PITFALLS: 6 (A 8-bit mode used for AND mask), 8 (Inherited mf=true for 
// field module byte-manipulation)
// HELPERS: next_char_emu(snes) — delegates NextChar @ $E7B8
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1004=1 (buffer start)
//   output_ram:  0x1004=1 (buffer modified)
//   entry_mode:  mf=true, xf=false, dp=0x1000, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::RemoveFloat ($85:DB)