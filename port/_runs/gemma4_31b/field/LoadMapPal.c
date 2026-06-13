#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0x0, DP=0
// This routine loads palette data for the map from the ROM into WRAM.
// It takes a palette index from $0FE0, divides it by 2, and uses it as 
// an offset to copy 32 bytes of palette data into two distinct WRAM regions.
static void LoadMapPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $0fe0 / sta $19 / stz $18 / lsr $19 / ror $18
    // This sequence performs a 16-bit right shift on the value at $0FE0.
    uint16_t pal_idx = ram[0x0FE0];
    uint16_t shifted_idx = pal_idx >> 1;

    // ldx $18 (X now holds the shifted index)
    uint16_t x = shifted_idx;
    // ldy #$0020
    uint16_t y = 0x20;

    // Loop @c209: Copies 32 bytes from two ROM tables based on X.
    // The loop runs until (y & 0x0F) == 0 AND y reaches a specific limit.
    // Note: The assembly uses 'tya / and #$0f / bne @c209' and then 
    // 'tya / adc #$10 / tay / bne @c209'. 
    // This effectively iterates y from 0x20 to 0x3F.
    while (y < 0x40) {
        // lda f:MapPal+$10,x / sta $0cdb,y
        // ROM accesses are handled via the emulator or mapped memory.
        // Since this is a translation, we assume a helper or direct access to ROM.
        // Here we simulate the ROM read from the MapPal table.
        uint8_t val1 = snes->rom_read(0xMapPal + 0x10 + x); 
        ram[0x0CDB + y] = val1;

        // lda f:MapPal+$90,x / sta $0ceb,y
        uint8_t val2 = snes->rom_read(0xMapPal + 0x90 + x);
        ram[0x0CEB + y] = val2;

        x++; // inx
        y++; // iny

        // tya / and #$0f / bne @c209
        if ((y & 0x0F) != 0) {
            continue;
        }

        // tya / clc / adc #$10 / tay / bne @c209
        // This is a complex way of checking if y has reached the end of the 32-byte block.
        // y is currently a multiple of 16 (because (y & 0x0f) == 0).
        // If y is 0x20, 0x30, etc., it continues. 
        // Once y reaches 0x40, the loop terminates.
        if (y < 0x40) {
            continue;
        }
    }
}

// PITFALLS: 6 (Mode A is 8-bit, but X/Y are 16-bit), 7 (Arithmetic truncation:
// the `and #$0f` and `adc #$10` sequence controls the loop boundaries).
// HELPERS: none (direct ROM access via hypothetical snes->rom_read)
//
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0FE0=1
//   output_ram:  0x0CDB=32, 0x0CEB=32
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::LoadMapPal ($C1:F9)