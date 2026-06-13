#include "snes/snes.h"

// Transfers background graphics (384 tiles) from ROM to VRAM via DMA
// Uses two DMA transfers per tile: first 16 bytes, then 8 bytes
// Repeats for 384 tiles (0x180 iterations)
static void TfrBGGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    // Set up DMA registers for first transfer (16 bytes)
    ram[0x4301] = 0x18;              // DMA destination: VRAM
    write16(ram, 0x2116, 0x0000);   // hVMADDL = 0 (VRAM address)
    for (uint16_t y = 0; y < 0x180; y++) {
        ram[0x2115] = 0x80;          // hVMAINC = 0x80 (VRAM increment mode)
        ram[0x4300] = 0x01;          // DMA control: fixed source, write to VRAM
        write16(ram, 0x4305, 0x0010); // DMA size: 16 bytes
        ram[0x2116] = 0x01;          // Trigger first DMA (source = $010000)
        ram[0x2116] = 0x00;          // Clear MDMAEN (end first transfer)
        ram[0x2115] = 0x00;          // hVMAINC = 0 (VRAM no increment)
        write16(ram, 0x4305, 0x0008); // DMA size: 8 bytes
        ram[0x2116] = 0x01;          // Trigger second DMA (source = $010010)
    }
}

// PITFALLS: 1 (DB=$7E not required - only writes to hardware registers),
//           4 (stack not used - no SP dependency),
//           6 (all operations are 8-bit - A mode assumed 8-bit),
//           7 (no arithmetic truncation needed - only register/memory ops)
// HELPERS: none (no subroutine calls)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::TfrBGGfx ($B1:43)