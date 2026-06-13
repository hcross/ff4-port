#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$E0, DP=0
// This routine initializes sprite palettes and a repeating sequence in RAM.
// 1. It copies either the "explosion palette" or the "default sprite palette"
//    based on the value at $0ACE.
// 2. It fills a 64-byte buffer at $0A6D with a repeating pattern of 0xFF, 0xFE, 0xFD, 0xFC.
static void field_00e075_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    ram[0xE5] = 0; // stz $e5
    
    // The value at $0ACD is shifted left by 5 (asl5 is a macro for 5 shifts)
    // and used as the starting offset for writing to $0DDB or $0DFB.
    uint8_t shift_val = ram[0xACD];
    uint8_t y = (uint8_t)(shift_val << 5); // asl5 / tay
    
    for (uint16_t x = 0; x < 0x10; x++) {   // ldx #0 / loop to 0x10
        if (ram[0xACE] == 0) {              // lda $0ace / bne @e092
            // Copy from MapSpritePal + (31 * 16)
            // MapSpritePal is a constant offset in the ROM/mapped area
            // For the purpose of the parity harness, we access the data directly
            uint8_t color = snes->rom[0xMapSpritePal + (31 * 16) + x]; 
            ram[0x0DDB + y] = color;        // sta $0ddb,y
        } else {
            // Copy from MapSpritePal + (32 * 16)
            uint8_t color = snes->rom[0xMapSpritePal + (32 * 16) + x];
            ram[0x0DFB + y] = color;        // sta $0dfb,y
        }
        y++; // iny
    }

    // Pattern fill: $0A6D to $0A6D + 0x3F
    for (uint16_t x = 0; x < 0x40; x++) {   // ldx #0 / loop to 0x40
        // x >> 1 then AND 3 then XOR FF:
        // This generates a sequence:
        // x=0: (0>>1)&3 ^ FF = 0 & 3 ^ FF = 0xFF
        // x=1: (1>>1)&3 ^ FF = 0 & 3 ^ FF = 0xFF
        // x=2: (2>>1)&3 ^ FF = 1 & 3 ^ FF = 0xFE
        // x=3: (3>>1)&3 ^ FF = 1 & 3 ^ FF = 0xFE
        // x=4: (4>>1)&3 ^ FF = 2 & 3 ^ FF = 0xFD
        // x=5: (5>>1)&3 ^ FF = 2 & 3 ^ FF = 0xFD
        // x=6: (6>>1)&3 ^ FF = 3 & 3 ^ FF = 0xFC
        // x=7: (7>>1)&3 ^ FF = 3 & 3 ^ FF = 0xFC
        uint8_t val = (uint8_t)(((x >> 1) & 0x03) ^ 0xFF);
        ram[0x0A6D + x] = val; // sta $0a6d,x
    }
}

// PITFALLS: 7 (Arithmetic truncation: ensuring the XOR and shift result in 8-bit values)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0ACD=1, 0x0ACE=1, MapSpritePal[0..31]=1
//   output_ram: 0x0A6D=64
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00e075 ($E0:75)