#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// The routine decodes a compressed tilemap from MoonTilemap into a temporary 
// buffer ($7F5C71), then replicates that buffer across three mirrors 
// ($7F5C31 source? No, it copies $7F5C31 to 71, B1, F1).
//
// Note: The ASM uses f:MoonTilemapPtrs which is in the ROM/FastRAM area.
// In this implementation, we treat those as offsets into the ROM/SRAM mapping.
static void DecodeMoonTilemap_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Initial pointer lookup
    uint16_t index = ram[0x3D]; // ldx $3d
    uint16_t ptr = read16(&snes->rom[0xMoonTilemapPtrs + index]); // lda f:MoonTilemapPtrs,x / sta $3d / lda +1,x / sta $3e
    
    uint16_t current_pos = ptr;
    uint16_t write_ptr = ram[0x40]; // ldx $40

    while (1) {
        uint8_t tile = snes->rom[0xMoonTilemap + current_pos]; // lda f:MoonTilemap,x
        
        if (tile & 0x80) { // bpl @fe87 is NOT taken (Negative/Bit 7 set)
            uint8_t count = (uint8_t)(tile & 0x7F); // and #$7f / pha
            uint8_t repeat_val = snes->rom[0xMoonTilemap + current_pos + 1]; // lda f:MoonTilemap+1,x
            
            uint8_t y = repeat_val + 1; // tay / iny
            
            while (y > 0) { // dey / bne @fe6f
                ram[0x7F5C71 + write_ptr] = count; // sta $7f5c71,x (A was restored from stack: pla)
                write_ptr++;
                y--;
            }
            
            ram[0x40] = write_ptr; // stx $40
            current_pos += 2;      // inx2 / stx $3d
        } else { // bpl @fe87 is taken
            ram[0x7F5C71 + write_ptr] = tile; // sta $7f5c71,x
            write_ptr++;
            ram[0x40] = write_ptr; // stx $40
            current_pos += 1;      // inx / stx $3d
        }

        if (write_ptr == 0x40) break; // cmp #$40 / beq @fe9d
    }

    // Final mirroring block
    ram[0x40] = write_ptr; 
    for (uint16_t x = 0; x < 0x40; x++) { // ldy #$40 / ldx $40 / dey / bne @fea2
        uint8_t val = ram[0x7F5C31 + x]; // lda $7f5c31,x
        ram[0x7F5C71 + x] = val;         // sta $7f5c71,x
        ram[0x7F5CB1 + x] = val;         // sta $7f5cb1,x
        ram[0x7F5CF1 + x] = val;         // sta $7f5cf1,x
    }
}

// PITFALLS: 6 (A is 8-bit, shifts/comparisons based on byte size), 
//           1 (DB is handled via absolute RAM offsets)
// HELPERS: read16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3D=1, 0x40=1
//   output_ram:  0x7F5C71=64
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DecodeMoonTilemap ($FE:4D)