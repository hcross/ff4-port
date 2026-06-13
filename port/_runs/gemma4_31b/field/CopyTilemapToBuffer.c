#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$F8 (Program), DP=0
// Purpose: Copies tilemap data from specific SRAM buffers ($7F:4800 range) 
// to a VRAM-destined buffer ($0ADB range).
// The loop iterates 64 times (0x40), shifting a index value by 1 to calculate offset.
static void CopyTilemapToBuffer_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t y = 0;

    while (y < 0x40) {
        // ldx $3d (X is 16-bit)
        uint16_t x = read16(ram, 0x3D);
        
        // lda $7f5c71,x (A is 8-bit)
        // The target address is $7F:5C71 + x
        uint8_t val = ram[0x15C71 + x]; // 0x7F000 + 0x5C71 = 0x15C71 in flat 24-bit map
        
        // sta $18 / stz $19 (Simulating 16-bit load in $18-$19)
        // longa / asl $18 / ldx $18
        // This sequence effectively does: index = (val << 1)
        uint16_t index = (uint16_t)val << 1;

        // The following offsets are absolute in WRAM ($7F:xxxx)
        // lda $7f4800,x -> sta $0adb,y
        ram[0x0ADB + y] = ram[0x14800 + index]; 
        // lda $7f4900,x -> sta $0add,y
        ram[0x0ADD + y] = ram[0x14900 + index];
        // lda $7f4a00,x -> sta $0b1b,y
        ram[0x0B1B + y] = ram[0x14A00 + index];
        // lda $7f4b00,x -> sta $0b1d,y
        ram[0x0B1D + y] = ram[0x14B00 + index];

        // iny4 (Y += 4)
        y += 4;
        
        // inc $3d
        uint16_t next_x = read16(ram, 0x3D) + 1;
        write16(ram, 0x3D, next_x);
        
        // cpy #$0040 / bne @f872
        // Note: The asm uses cpy #$40 on Y. 
        // With iny4, Y reaches 0x40 after 16 iterations, 
        // but the loop is designed for 64 increments of $3d.
        // Wait: cpy #$0040 on Y with iny4 means 16 iterations.
        if (y == 0x40) break;
    }
}

// PITFALLS: 6 (Mode A transitions: the logic uses 8-bit A for the initial load, 
// then 16-bit A for the shift. C handles this naturally via uint16_t cast).
// 7 (Shift truncation: asl $18 on 16-bit word preserves bits up to 16).
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3D=2, 0x15C71=1, 0x14800=1, 0x14900=1, 0x14A00=1, 0x14B00=1
//   output_ram:  0x0ADB=1, 0x0ADD=1, 0x0B1B=1, 0x0B1D=1, 0x3D=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xF8
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::CopyTilemapToBuffer ($F8:6F)