#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7F (target address $7F5CE7), DP=0
// This routine modifies the overworld tilemap based on event switches and current position ($93).
// Note: The ASM uses indirect-indexed addressing (e.g., sta $7f5ce7,x).
// In this context, X is loaded from $3d, which was cleared to 0, so X = 0.
static void ModOverworldTilemap_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Initial setup
    uint8_t pos = ram[0x93];
    ram[0x3E] = pos & 0x3F;
    ram[0x3D] = 0;
    uint16_t x = (uint16_t)ram[0x3D]; // ldx $3d

    // Check event switch $0c
    if ((ram[0x1282] & 0x10) != 0) {
        if (pos == 0x39) { // destroyed damcyan (118,57)
            ram[0x5CE7 + x] = 0x2D;
            ram[0x5CE8 + x] = 0x2E; // inc / sta
            return;
        } else if (pos == 0x3A) {
            ram[0x5CE7 + x] = 0x3D;
            ram[0x5CE8 + x] = 0x3E; // inc / sta
            return;
        }
    }

    // Check event switch $0e
    if ((ram[0x1281] & 0x40) != 0) {
        if (pos == 0x76) { // mist mountains (97,118)
            ram[0x5CD2 + x] = 0x13;
            ram[0x5CD3 + x] = 0x13;
            ram[0x5CD4 + x] = 0x13;
            ram[0x5CD5 + x] = 0x13;
            ram[0x5CD6 + x] = 0x13;
            return;
        } else if (pos == 0x77) {
            ram[0x5CD2 + x] = 0x12;
            ram[0x5CD3 + x] = 0x13;
            ram[0x5CD4 + x] = 0x13;
            ram[0x5CD5 + x] = 0x13;
            ram[0x5CD6 + x] = 0x14;
            return;
        } else if (pos == 0x78) {
            ram[0x5CD2 + x] = 0x13;
            ram[0x5CD3 + x] = 0x13;
            ram[0x5CD4 + x] = 0x13;
            ram[0x5CD5 + x] = 0x13;
            ram[0x5CD6 + x] = 0x13;
            return;
        }
    }

    // Check event switch $30
    if ((ram[0x1286] & 0x01) == 0) { // bne @c7e4 means "if bit set, skip"
        if (pos == 0xD2) { // agart hole to underground (106,210)
            ram[0x5CDB + x] = 0x13;
        } else if (pos == 0xD3) {
            ram[0x5CDA + x] = 0x13;
            ram[0x5CDB + x] = 0x13;
            ram[0x5CDC + x] = 0x13;
        } else if (pos == 0xD4) {
            ram[0x5CD9 + x] = 0x13;
            ram[0x5CDA + x] = 0x13;
            ram[0x5CDB + x] = 0x13;
            ram[0x5CDC + x] = 0x13;
            ram[0x5CDD + x] = 0x13;
        } else if (pos == 0xD5) {
            ram[0x5CDA + x] = 0x13;
            ram[0x5CDB + x] = 0x13;
            ram[0x5CDC + x] = 0x13;
        } else if (pos == 0xD6) {
            ram[0x5CDB + x] = 0x13;
        }
    }
}

// PITFALLS: 1 (Target memory $7F5C.. requires DB=0x7F or absolute indexing), 
// 6 (A is 8-bit), 8 (mf=true inherited)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x93=1, 0x1282=1, 0x1281=1, 0x1286=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7F
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::ModOverworldTilemap ($C6:00F4)