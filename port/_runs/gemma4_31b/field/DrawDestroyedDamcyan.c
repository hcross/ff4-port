#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$D7, DP=0
// Logic:
//   Check if ram[0x2C] == 0x60. If not, return.
//   Otherwise, clear $2115 and loop 16 times to write VRAM data for 
//   destroyed damcyan tiles via I/O ports $2116, $2117, $2118.
static void DrawDestroyedDamcyan_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0x2C] != 0x60) { // lda $2c / cmp #$60 / bne @d798
        return;
    }

    // Clear VRAM address low byte
    snes->io[0x2115] = 0; 

    for (uint16_t x = 0, y = 0; x < 0x10; x += 4, y += 2) {
        // The ASM uses indices based on the labels DestroyedDamcyanVRAMTbl and DestroyedDamcyanTiles.
        // These are typically in ROM or a fixed data section.
        
        // lda DestroyedDamcyanVRAMTbl,y / sta $2116
        snes->io[0x2116] = snes->rom[DestroyedDamcyanVRAMTbl + y];
        // lda DestroyedDamcyanVRAMTbl+1,y / sta $2117
        snes->io[0x2117] = snes->rom[DestroyedDamcyanVRAMTbl + 1 + y];

        // The ASM sequence writes 4 bytes to the same address $2118 in succession.
        // Only the final write is observable in a standard emulator, but we mirror the sequence.
        // lda f:DestroyedDamcyanTiles,x / sta $2118
        snes->io[0x2118] = snes->rom[DestroyedDamcyanTiles + x];
        // lda f:DestroyedDamcyanTiles+1,x / sta $2118
        snes->io[0x2118] = snes->rom[DestroyedDamcyanTiles + 1 + x];
        // lda f:DestroyedDamcyanTiles+2,x / sta $2118
        snes->io[0x2118] = snes->rom[DestroyedDamcyanTiles + 2 + x];
        // lda f:DestroyedDamcyanTiles+3,x / sta $2118
        snes->io[0x2118] = snes->rom[DestroyedDamcyanTiles + 3 + x];
    }
}

// PITFALLS: None relevant for this routine (direct I/O and ROM reads).
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2C=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xD7
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Writes to I/O ports $2115-$2118, requires I/O monitoring)

// REVERSED_FUNCTION: field::DrawDestroyedDamcyan ($D7:58)