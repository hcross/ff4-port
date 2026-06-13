#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0x00, DP=0x00
// Purpose: Copies palette data from MapSpritePal into a target buffer at $0DDB.
// The routine uses a strange nested loop structure that iterates in blocks 
// of 16 bytes, clearing trailing bytes until a total of 0x80 bytes are processed.
static void LoadPlayerPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = 0;
    uint16_t y = 0;

    while (y != 0x0080) {
        // First loop: Copy 16 bytes from MapSpritePal to $0DDB+y
        do {
            ram[0x0DDB + y] = ram[0xMapSpritePal + x]; // lda f:MapSpritePal,x / sta $0ddb,y
            x++;
            y++;
            if ((y & 0x0F) != 0) { // tya / and #$0f / bne @c82d
                continue; 
            }
            break;
        } while (0);

        // Second loop: Fill with 0s until the next 16-byte boundary
        do {
            ram[0x0DDB + y] = 0; // lda #0 / sta $0ddb,y
            y++;
            if ((y & 0x0F) != 0) { // tya / and #$0f / bne @c83b
                continue;
            }
            break;
        } while (0);

        // Check if 0x80 bytes have been processed
        if (y == 0x0080) { // cpy #$0080 / bne @c82d
            break;
        }
    }
}

// PITFALLS: 6 (A 8-bit mode for palette bytes), 8 (Inherited mode: mf=true, xf=false)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xMapSpritePal=1 (array)
//   output_ram:  0x0DDB=1 (block of 0x80 bytes)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadPlayerPal ($C8:27)