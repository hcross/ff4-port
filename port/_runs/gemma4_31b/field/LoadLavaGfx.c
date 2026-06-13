#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (Hardware/IO), DP=0
// Logic:
//   1. Sets DMA destination address to $7F5800 (via $2115 and $2116).
//   2. Sets DMA control register $2115 to $80 (likely triggering DMA or setting a flag).
//   3. Manually copies 256 bytes from $213A (likely a window into VRAM or a buffer)
//      to the address $7F5800.
// Note: While $2115/$2116 are usually DMA registers, the routine performs 
// a manual CPU loop to copy data from $213A to RAM.
static void LoadLavaGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Hardware register writes
    // $2115 and $2116 are outside WRAM; accessed via snes->io or similar
    // In this context, we treat them as writes to the emulator's hardware state.
    snes->io[0x15] = 0x80; 
    snes->io[0x16] = 0x00; // low byte of $3800
    snes->io[0x17] = 0x38; // high byte of $3800

    // Manual copy loop
    // The ASM loads from $213A (hardware register/window) and stores to $7F5800
    for (uint16_t x = 0; x < 0x0100; x++) {
        ram[0x5800 + x] = snes->io[0x3A]; 
    }
}

// PITFALLS: 1 (DB=0 for IO access), 6 (A is 8-bit, X is 16-bit)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x5800=1 (range 0x5800-0x58FF)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::LoadLavaGfx ($8E:C4)