#include "snes/snes.h"

// Logic:
// This routine iterates through a palette table (256 words), 
// inverts the color bits (EOR $7FFF), masks specific bits (AND $7BDE),
// and shifts the result right by 1.
//
// Entry mode: A/X inherited from caller.
// Routine explicitly sets A=16-bit (longa), X=16-bit (via inx2/cpx #$100).
// DP=$0C, DB=$00 (based on addresses $0CDB and $0BDB).
static void InvertPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = 0;

    do {
        // lda $0cdb,x (16-bit)
        uint16_t val = read16(ram, 0x0CDB + x);
        
        // eor #$7fff / and #$7bde
        val ^= 0x7FFF;
        val &= 0x7BDE;
        
        // lsr (16-bit)
        val >>= 1;
        
        // sta $0bdb,x (16-bit)
        write16(ram, 0x0BDB + x, val);
        
        x += 2; // inx2
    } while (x != 0x0100); // cpx #$0100 / bne

    snes->cpu->a = 0;      // lda #0
}

// PITFALLS: 6 (Routine uses longa, so all operations on 'val' and 
// memory access are 16-bit). 8 (A is restored to 8-bit via shorta 
// before return, though not explicitly changing a C variable).
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0CDB=1, 0x0CDC=1 (and subsequent 255 bytes)
//   output_ram:  0x0BDB=1 (and subsequent 255 bytes)
//   entry_mode:  mf=true, xf=false, dp=0x0C, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::InvertPal ($C6:D5)