#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C2, DP=0
// Logic:
//   1. Calls GetNPCMapPtr to retrieve a pointer (offset) into the NPC map.
//   2. Reads the result from DP:0x3D.
//   3. Writes 0 to the address at $7F:4C00 + offset.
// Note: The ASM uses absolute addressing $7f4c00, which is outside 
// the standard WRAM bank range ($7E:0000-$7F:FFFF) and likely 
// targets a specific memory-mapped region in the game's address space.
static void ClearNPCMap_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // phx / plx : X is preserved across the routine
    get_npc_map_ptr_emu(snes); 

    // ldx $3d : reads the offset returned by GetNPCMapPtr
    // Since DP=0, this is ram[0x3D]. 
    // Based on the ASM, X is used as an index (16-bit relative to $7F4C00).
    uint16_t offset = read16(ram, 0x3D); 

    // lda #0 / sta $7f4c00,x
    // In C, $7F:4C00 is an absolute address. 
    // Mapping: $7F0000 is 0x1E000 in the 128KB WRAM array (0x7E0000 is 0).
    // $7F4C00 = 0x7E0000 + 0x14C00.
    uint32_t target_addr = 0x14C00 + offset;
    ram[target_addr] = 0;
}

// PITFALLS: None (straightforward store)
// HELPERS: get_npc_map_ptr_emu(snes) — delegates GetNPCMapPtr @ $C3:55
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3D=2
//   output_ram:  0x14C00=1 (offset by read16(0x3D))
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC2
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ClearNPCMap ($C2:00FD)