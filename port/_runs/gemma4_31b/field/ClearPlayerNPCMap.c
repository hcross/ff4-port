#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$AB, DP=0
// Purpose: Clear NPC map data, conditionally skipping if the player is on 
// a bridge tile that doesn't match the current bridge ID in $D2.
static void ClearPlayerNPCMap_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $a1 / and #$04 / beq @ab21
    if ((ram[0xA1] & 0x04) != 0) {
        // On a bridge tile: check if bridge ID matches ram[$d2]
        // lda $a1 / and #$03 / and $d2 / beq @ab2e
        if ((ram[0xA1] & 0x03 & ram[0xD2]) == 0) {
            return; // beq @ab2e
        }
    }

    // @ab21: Setup coordinates for ClearNPCMap
    // lda $1706 / sta $0c
    ram[0x0C] = ram[0x1706];
    // lda $1707 / sta $0e
    ram[0x0E] = ram[0x1707];

    // jsr ClearNPCMap (delegated)
    ClearNPCMap_emu(snes);
}

// PITFALLS: 1 (DB=$AB required for field module RAM access)
// HELPERS: ClearNPCMap_emu(snes) — delegates ClearNPCMap @ $C2FD
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00A1=1, 0x00D2=1, 0x1706=1, 0x1707=1
//   output_ram:  0x000C=1, 0x000E=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xAB
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ClearPlayerNPCMap ($AB:13)