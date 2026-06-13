#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7F, DP=0
// This routine transfers 8 bytes from a source table in bank $7F (offset by X)
// to the VRAM data register ($2119).
static void TfrWaterTiles_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = snes->cpu->x;
    
    // $7f5800 maps to ram[0x10000 + 0x5800] = 0x15800
    uint32_t base_addr = 0x15800;

    // The assembly performs 8 consecutive load-store operations to the same IO register.
    // In the parity harness, IO writes are reflected in ram[0x2119].
    for (int i = 0; i < 8; i++) {
        ram[0x2119] = ram[base_addr + x + i];
    }
}

// PITFALLS: 1 (DB=$7F for source table access)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x15800=1, 0x15801=1, 0x15802=1, 0x15803=1, 0x15804=1, 0x15805=1, 0x15806=1, 0x15807=1
//   output_ram:  0x2119=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7F
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::TfrWaterTiles ($8E:0E)