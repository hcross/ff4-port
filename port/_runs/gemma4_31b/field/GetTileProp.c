#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9F, DP=0
// This routine determines the tile property for a given coordinate.
// It handles standard map wrapping (mod 64) or sub-map boundary checks.
//
// The tilemap and property tables are in ROM. In the LakeSnes/snesrev 
// architecture, ROM is accessed via snes->rom[address].
static void GetTileProp_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    const uint8_t *rom = snes->rom;

    if (ram[0x1700] != 3) { // cmp #3 / beq @9fce
        // Normal map: Y is mod 64
        ram[0x3E] = ram[0x1B] & 0x3F;
        ram[0x3D] = ram[0x1A];
    } else {
        // Sub-map: Bounds check 0x00-0x1F
        int8_t x = (int8_t)ram[0x1A];
        int8_t y = (int8_t)ram[0x1B];

        if (x < 0 || x >= 0x20 || y < 0 || y >= 0x20) { // bmi / bcs / bmi / bcc
            // Out of bounds
            if ((int8_t)ram[0x0FDF] >= 0) { // bpl @9fe9
                write16(ram, 0x1E, 7);
            } else {
                write16(ram, 0x1E, 0);
            }
            return;
        }
        // In bounds
        ram[0x3E] = (uint8_t)y;
        ram[0x3D] = (uint8_t)x;
    }

    // Tilemap lookup
    uint8_t tile = rom[0x7F5C71 + ram[0x3D]];
    ram[0x06] = tile;
    ram[0x18] = tile;
    ram[0x19] = 0;

    // Calculate property index: (tile << 1)
    // Pitfall 7: Ensure 8-bit logic for the shift result
    uint16_t prop_idx = (uint16_t)tile << 1;

    // Property lookup from ROM table at 0x0EDB
    // 65816: lda $0edb,x / sta $1e / lda $0edc,x / sta $1f
    uint16_t prop = (uint16_t)rom[0x0EDB + prop_idx] | ((uint16_t)rom[0x0EDC + prop_idx] << 8);
    write16(ram, 0x1E, prop);
}

// PITFALLS: 1 (Bank $9F for ROM/RAM context), 3 (BCS/BCC inversion for bounds), 
// 7 (Shift truncation handled via 16-bit cast for index)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1700=1, 0x1A=1, 0x1B=1, 0x0FDF=1
//   output_ram:  0x1E=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9F
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetTileProp ($9F:C0)