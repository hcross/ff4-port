#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$AA, DP=0
// This routine determines if the player can pass through a tile.
// It checks for running events, NPC blocks, and bridge/z-level constraints.
// The result is returned in A (1 = Pass, 0 = Blocked).
static void CheckTilePass_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0xB1] != 0) {                 // lda $b1 / beq @aa5f (BNE is implicit)
        snes->cpu->a = 0;
        return;
    }

    // jsr CheckNPCBlock / cmp #$00 / bne @aa91
    // Note: CheckNPCBlock_emu returns the value of A after RTS
    uint16_t npc_res = CheckNPCBlock_emu(snes);
    if (npc_res != 0) {
        snes->cpu->a = 1;
        return;
    }

    // lda $0709 / asl / tay
    uint8_t y = (uint8_t)(ram[0x0709] << 1); // Pitfall 7: 8-bit truncation

    // lda $a1 / and #$04 / beq @aa7d
    if (ram[0xA1] & 0x04) {               // Bridge tile
        uint8_t tile_val = ram[0x06A1 + y];
        // lda $06a1,y / and #$03 / and $d2 / beq @aa87
        if ((tile_val & 0x03) & ram[0xD2]) {
            snes->cpu->a = 0;
            return;
        }
        // lda $06a1,y / and #$04 / beq @aa91
        if (tile_val & 0x04) {
            snes->cpu->a = 0;
            return;
        }
    } else {                             // Not bridge tile (@aa7d)
        uint8_t tile_val = ram[0x06A1 + y];
        // lda $06a1,y / and $0a / beq @aa91
        if (tile_val & 0x0A) {
            snes->cpu->a = 0;
            return;
        }
    }

    snes->cpu->a = 1; // @aa91: lda #$01
}

// PITFALLS: 7 (8-bit ASL truncation via uint8_t cast)
// HELPERS: CheckNPCBlock_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0B1=1, 0x0709=1, 0x0A1=1, 0x0D2=1, 0x06A1=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xAA
//   entry_flags: z=auto, n=auto
//   returns: uint8_t in snes->cpu->a
// REVERSED_FUNCTION: field::CheckTilePass ($AA:58)