#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0, DP=0
// Logic:
//   Copies 16 bytes from MapSpritePal offset (33 * 16) to $0E1B and $0AAD
//   Copies 16 bytes from MapSpritePal offset (34 * 16) to $0E3B
//
// Note: MapSpritePal is in the ROM/Bank area (f: prefix), 
// but the output is written to WRAM.
static void LoadWhirlpoolPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // MapSpritePal is located in ROM. In this harness, ROM access 
    // is typically handled via the emulator's memory map or a 
    // ROM pointer. Based on the 'f:' prefix, we access the source bytes.
    // MapSpritePal offsets: 33*16 = 0x210, 34*16 = 0x220
    const uint8_t *rom_base = snes->rom; 
    // Note: Actual ROM address of MapSpritePal is required. 
    // Assuming the harness provides the resolved address for the 'f:' label.
    const uint8_t *src_pal1 = &snes->rom[0xMapSpritePal + 0x210]; 
    const uint8_t *src_pal2 = &snes->rom[0xMapSpritePal + 0x220];

    for (uint16_t x = 0; x < 0x10; x++) { // ldx #0 / cpx #$0010
        uint8_t val1 = src_pal1[x];       // lda f:MapSpritePal+33*16,x
        ram[0x0E1B + x] = val1;          // sta $0e1b,x
        ram[0x0AAD + x] = val1;          // sta $0aad,x
        
        uint8_t val2 = src_pal2[x];       // lda f:MapSpritePal+34*16,x
        ram[0x0E3B + x] = val2;          // sta $0e3b,x
    }
}

// PITFALLS: None (Simple memory copy, no conditional branches on entry, 
//            A is 8-bit, X is 16-bit).
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0E1B=16, 0x0AAD=16, 0x0E3B=16
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadWhirlpoolPal ($D2:93)