#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Input:  X = offset into $2000 table (16-bit index)
// Output: $4F = moon tile index (8-bit)
static void GetMoonTile_c(Snes *snes, uint16_t x_index) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    ram[0x28] = 0;
    uint8_t a2100 = ram[0x2100];
    if (a2100 != 0) {           // beq _e2ed
        ram[0x28] = 0x10;
    }

    // longa: A now 16-bit
    cpu->mf = false;
    uint16_t val = read16(ram, 0x2000 + x_index);  // lda $2000,x (16-bit)
    val &= 0x01FF;             // and #$01ff
    val >>= 5;                 // lsr5 (5x lsr)
    write16(ram, 0x2A, val);   // sta $2a

    // shorta0: A = 0 then 8-bit
    cpu->mf = true;
    uint8_t a2a = ram[0x2A];   // lda $2a (low byte)
    a2a += ram[0x28];          // clc / adc $28
    uint16_t x_reg = a2a;      // tax (X now 16-bit per xf=0)
    uint8_t tile = ram[0x8AEA + x_reg]; // lda f:MoonTileTbl,x (bank E2 assumed)
    tile <<= 2;                // asl2 (2x asl)
    tile += 0x10;              // clc / adc #$10
    ram[0x4F] = tile;          // sta $4f
}

// PITFALLS: 1 (DB=$7E assumed for data access), 6 (mode A 8/16-bit transitions),
//           7 (arithmetic truncation in 8-bit mode), 8 (inherited mf/xf flags)
// HELPERS: read16/write16 for 16-bit memory access
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x2000=2, 0x2100=1
//   output_ram:  0x4F=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::GetMoonTile ($E2:E2)