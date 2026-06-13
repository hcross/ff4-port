#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 8-bit (xf=1), DB=0, DP=0
// This routine copies the window palette from ROM to VRAM/registers 
// and updates a palette index.
//
// Logic:
//   1. Copy 32 bytes from f:WindowPal to address $0CDB
//   2. Read 16-bit value from $16AA and write it to $0CDD
static void LoadWindowPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Assuming f:WindowPal is a fixed ROM address defined in the project's 
    // memory map. Based on typical FF4 layouts, this is a ROM read.
    // For the parity harness, we treat it as a read from the ROM image.
    const uint8_t *window_pal = &snes->rom[0xWindowPal_Offset]; // Abstracted ROM offset

    // Loop @c229: Copy 32 bytes
    for (uint8_t x = 0; x < 0x20; x++) {
        ram[0x0CDB + x] = window_pal[x];
    }

    // Update palette pointer/index
    uint16_t val = read16(ram, 0x16AA);
    write16(ram, 0x0CDD, val);
}

// PITFALLS: None relevant (simple copy and 16-bit transfer)
// HELPERS: read16/write16 — little-endian 16-bit accessors over ram[]
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x16AA=2, WindowPal=32 (ROM)
//   output_ram:  0x0CDB=32, 0x0CDD=2
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadWindowPal ($C2:26)