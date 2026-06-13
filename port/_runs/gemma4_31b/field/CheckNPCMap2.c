#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C3, DP=0
// Logic:
//   Checks if NPC coordinates at DP:$0C (x) and DP:$0E (y) are within [0, 0x1F].
//   If both are < 0x20, it fetches the map pointer and returns the value 
//   at ROM address $7F4C00 + offset.
//   Otherwise, returns 0 in A.
static void CheckNPCMap2_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // lda $0c / cmp #$20 / bcs @c341
    if (ram[0x0C] >= 0x20) { // Pitfall 3: bcs branches when A >= 0x20
        snes->cpu->a = 0;
        return;
    }
    
    // lda $0e / cmp #$20 / bcs @c341
    if (ram[0x0E] >= 0x20) { // Pitfall 3: bcs branches when A >= 0x20
        snes->cpu->a = 0;
        return;
    }

    // jsr GetNPCMapPtr
    GetNPCMapPtr_emu(snes);
    
    // ldx $3d (DP=0, X is 16-bit)
    uint16_t x = read16(ram, 0x3D);
    
    // lda $7f4c00,x (FastROM access)
    // The harness provides snes->rom as a pointer to the ROM image.
    // $7F4C00 is the absolute address in the SNES memory map.
    snes->cpu->a = snes->rom[0x7F4C00 + x];
}

// PITFALLS: 3 (CMP/BCS inversion: bcs branches when A >= 0x20, so we
// enter the 'return 0' path when A >= 0x20)
// HELPERS: GetNPCMapPtr_emu(snes) — delegates GetNPCMapPtr @ $C3:55
//          read16 — little-endian 16-bit access to DP:$3D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0C=1, 0x0E=1, 0x3D=2
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC3
//   entry_flags: z=auto, n=auto
//   return_reg:  a=8
// REVERSED_FUNCTION: field::CheckNPCMap2 ($C3:28)