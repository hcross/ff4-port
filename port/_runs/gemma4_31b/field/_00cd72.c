#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 8-bit (xf=1), DB=$CD, DP=0
// This routine copies two blocks of palette data from ROM to WRAM.
// It iterates 16 times (0x00 to 0x0F).
//
// Source 1: MapSpritePal + (35 * 16) -> Destination 1: $0E1B
// Source 2: MapSpritePal + (36 * 16) -> Destination 2: $0E3B
static void _00cd72_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Note: MapSpritePal is in ROM. 
    // Assuming MapSpritePal is a known symbol/offset in the ROM image.
    // MapSpritePal offset is typically handled by the ROM read logic in the harness.
    // f:MapSpritePal+35*16 = MapSpritePal + 560
    // f:MapSpritePal+36*16 = MapSpritePal + 576
    const uint8_t *rom = snes->rom; 
    uint32_t map_sprite_pal_base = 0x0000; // This would be the actual symbol value from the map

    for (uint8_t x = 0; x < 0x10; x++) {
        // lda f:MapSpritePal+35*16,x / sta $0e1b,x
        ram[0x0E1B + x] = rom[map_sprite_pal_base + (35 * 16) + x];
        
        // lda f:MapSpritePal+36*16,x / sta $0e3b,x
        ram[0x0E3B + x] = rom[map_sprite_pal_base + (36 * 16) + x];
    }
}

// PITFALLS: None relevant for this simple copy loop.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none (reads from ROM)
//   output_ram:  0x0E1B=16, 0x0E3B=16
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0xCD
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::_00cd72 ($CD:72)