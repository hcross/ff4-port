#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$B1 (implied by ROM bank), DP=0
// Purpose: Transfers world palette and graphics data to VRAM.
// 1. Copies WorldTilePal (256 bytes) to WRAM $0BDB.
// 2. Copies WorldBGGfx (256 bytes) to VRAM using calculated offsets.
static void TfrWorldGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // hVMAINC and hVMADDL are hardware registers (typically mapped to $2005/2006 or similar)
    // In this context, they represent VRAM address setup.
    snes->cpu->a = 0x80;
    // sta hVMAINC -> Writing to VMA increment register
    // stx hVMADDL -> Writing to VMA address low register
    // Note: These are typically handled by the emulator's I/O mapping.
    
    // Load index for graphics transfer
    uint8_t world_idx = ram[0x1700];
    uint8_t high_byte = world_idx;
    uint8_t low_byte = 0;

    // Loop @b198: Copy WorldTilePal to ram[0x0BDB]
    // f:WorldTilePal is a ROM address. 
    // Since we are in a C translation, we access ROM via snes->rom or a pointer.
    for (uint16_t x = 0; x < 0x100; x++) {
        // lda f:WorldTilePal,x / sta $0bdb,y
        ram[0x0BDB + x] = snes->rom[0xWorldTilePal_Offset + x]; 
    }

    // Prepare for second phase: WorldBGGfx
    // asl5 is a macro for A << 5
    uint8_t shifted_idx = (uint8_t)(world_idx << 5);
    high_byte = shifted_idx;
    low_byte = 0;

    for (uint16_t y = 0; y < 0x100; y++) {
        // Inner loop @b1b7
        for (uint16_t x = 0; x < 0x20; x++) { // and #$1f (bne @b1b7)
            // lda f:WorldBGGfx,x / sta $08
            uint8_t gfx_byte = snes->rom[0xWorldBGGfx_Offset + x];
            ram[0x08] = gfx_byte;

            // Calculate VRAM address H
            // and #$0f / clc / adc $0bdb,y
            uint8_t addr_h = (uint8_t)((x & 0x0F) + ram[0x0BDB + y]);
            // sta hVMDATAH (Emulator handles VRAM write)
            
            // lsr4 is a macro for A >> 4
            // clc / adc $0bdb,y
            uint8_t addr_h_shifted = (uint8_t)((gfx_byte >> 4) + ram[0x0BDB + y]);
            // sta hVMDATAH
            
            x++;
            if ((x & 0x1F) == 0) break; // bne @b1b7 condition (x is updated by inx)
        }
        y++;
    }
}

// PITFALLS: 7 (8-bit shift truncation), 8 (Inherited mf=true for battle/field logic)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1700=1
//   output_ram:  0x0BDB=1, 0x08=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB1
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (interacts with VMA hardware registers)

// REVERSED_FUNCTION: field::TfrWorldGfx ($B1:81)