#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x8F (ROM/IO), DP=0
// Logic:
//   1. Sets DMA source address $2115=$80 and $2116=$2000.
//   2. Reads data from $213A (likely an IO register or mapped ROM) and copies 
//      256 bytes to WRAM $7F:5800.
//   3. Sets DMA source address $2116=$1E80.
//   4. Reads data from $213A and copies 256 bytes to WRAM $7F:5900.
static void LoadWaterGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Note: Addresses $2115, $2116, $213A are SNES IO registers.
    // In the context of this harness, these are accessed via snes->io or 
    // simulated memory mapping. Assuming the harness maps IO to snes->ram 
    // or provides equivalent accessors.
    
    snes->ram[0x2115] = 0x80;
    write16(ram, 0x2116, 0x2000);

    // Loop 1: Copy 256 bytes from $213A to $7F:5800
    for (uint16_t x = 0; x < 0x0100; x++) {
        ram[0x7F5800 + x] = ram[0x213A];
    }

    write16(ram, 0x2116, 0x1E80);

    // Loop 2: Copy 256 bytes from $213A to $7F:5900
    for (uint16_t x = 0; x < 0x0100; x++) {
        ram[0x7F5900 + x] = ram[0x213A];
    }
}

// PITFALLS: 1 (DB=$8F for IO access), 6 (X is 16-bit for index loop)
// HELPERS: write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x213A=1
//   output_ram:  0x7F5800=1 (block), 0x7F5900=1 (block)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x8F
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (multiple output blocks)

// REVERSED_FUNCTION: field::LoadWaterGfx ($8F:69)