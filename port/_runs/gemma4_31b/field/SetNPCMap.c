#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C3 (mapped via snes->ram), DP=0
// Purpose: Sets a specific flag (bit 7) in the NPC map data for the current NPC.
// Logic:
//   1. Get the pointer/index to the NPC map via GetNPCMapPtr.
//   2. Read the index from DP:$3D.
//   3. Read the current map byte from DP:$AE.
//   4. Set bit 7 (ORA #$80) and write it back to the NPC map array.
static void SetNPCMap_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // phx is handled implicitly as we don't modify the X register across the call
    get_npc_map_ptr_emu(snes); 

    // ldx $3d
    uint16_t x = ram[0x3D]; 

    // lda $ae
    uint8_t a = ram[0xAE];

    // ora #$80
    a |= 0x80;

    // sta $7f4c00,x
    // Note: $7F4C00 is an absolute address. In snesrev/LakeSnes, 
    // WRAM mapping typically treats $7F0000 as offset 0x10000.
    // 0x7F4C00 maps to 0x14C00 within the 128KB WRAM range.
    ram[0x14C00 + x] = a;
}

// PITFALLS: 1 (Address $7F4C00 is WRAM high bank, offset 0x14C00), 6 (A is 8-bit)
// HELPERS: get_npc_map_ptr_emu(snes) — delegates GetNPCMapPtr @ $C3:55
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3D=1, 0xAE=1
//   output_ram:  0x14C00=1 (indexed by ram[0x3D])
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC3
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::SetNPCMap ($C3:45)