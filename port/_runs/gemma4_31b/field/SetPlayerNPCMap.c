#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$AA (Program Bank), DP=0
// Purpose: Updates the NPC map position of the player based on their current 
//          movement state, unless they are on a bridge tile with specific flags.
//
// Logic:
//   1. Check if player is on a bridge tile using the current move index.
//   2. If on a bridge AND specific flags are met, skip NPC map update.
//   3. Otherwise, calculate new X/Y coordinates using movement tables 
//      relative to player's current position ($1706/$1707).
//   4. Set player ID ($FF) and call SetNPCMap to commit to memory.
static void SetPlayerNPCMap_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // lda $ab / asl / tay
    uint8_t move_idx = ram[0xAB];
    uint8_t y = (uint8_t)(move_idx << 1); // Pitfall 7: wrap to 8-bit

    // lda $06a1,y / and #$04 / beq @aaec
    // Note: $06a1 is in DP=0, so it's ram[0x06a1 + y]
    uint8_t tile_attr = ram[0x06A1 + y];
    if ((tile_attr & 0x04) == 0) {
        goto label_aaec;
    }

    // lda $06a1,y / and #$03 / and $d2 / beq @ab08
    // Note: $d2 is a direct page address (ram[0x00D2])
    uint8_t bridge_check = (tile_attr & 0x03) & ram[0x00D2];
    if (bridge_check == 0) {
        goto label_ab08;
    }

label_aaec:;
    // lda $ab / tay
    y = ram[0xAB];
    
    // lda $1706 / clc / adc XMoveTbl,y / sta $0c
    // XMoveTbl and YMoveTbl are absolute addresses in the ROM/RAM mapping
    // These tables are typically in ROM, but accessed here via the harness.
    uint8_t posX = ram[0x1706];
    uint8_t offsetX = ram[0xXMoveTbl + y]; // XMoveTbl is a constant address
    ram[0x0C] = (uint8_t)(posX + offsetX);

    // lda $1707 / clc / adc YMoveTbl,y / sta $0e
    uint8_t posY = ram[0x1707];
    uint8_t offsetY = ram[0xYMoveTbl + y]; // YMoveTbl is a constant address
    ram[0x0E] = (uint8_t)(posY + offsetY);

    // lda #$ff / sta $ae / jsr SetNPCMap
    ram[0xAE] = 0xFF;
    set_npc_map_emu(snes);

label_ab08:;
    return;
}

// PITFALLS: 7 (ASL 8-bit truncation)
// HELPERS: set_npc_map_emu(snes) — delegates SetNPCMap @ $C345
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xAB=1, 0x06A1=1, 0x00D2=1, 0x1706=1, 0x1707=1, XMoveTbl=1, YMoveTbl=1
//   output_ram:  none (writes to 0x0C, 0x0E, 0xAE and calls SetNPCMap)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xAA
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::SetPlayerNPCMap ($AA:D8)