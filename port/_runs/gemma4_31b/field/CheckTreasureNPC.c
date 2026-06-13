#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E (WRAM), DP=0
// This routine checks if the player is in a state allowing treasure or NPC interaction.
// It validates several status flags (B1, 5A, 5C) and checks for the A-button press.
// If the map window is open (EA), it closes it before processing interactions.
static void CheckTreasureNPC_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0xB1] != 0) return;       // lda $b1 / bne @9876
    if ((ram[0x5A] & 0x0F) != 0) return; // lda $5a / and #$0f / bne @9876
    if ((ram[0x5C] & 0x0F) != 0) return; // lda $5c / and #$0f / bne @9876

    // JOY_A typically defined as 0x80 or 0x01 depending on header; 
    // matching the logic: branch if (ram[0x02] & JOY_A) != 0
    if ((ram[0x02] & 0x80) == 0) {    // lda $02 / and #JOY_A / bne @9863
        return;
    }

    if (ram[0x54] != 0) {             // lda $54 / beq @9868
        return;
    }

    ram[0x54]++;                      // inc $54
    if (ram[0xEA] == 0) {             // lda $ea / bne @9870
        ram[0xEA]++;                  // inc $ea
    }

    check_treasure_emu(snes);         // jsr CheckTreasure
    check_npcs_emu(snes);             // jsr CheckNPCs
}

// PITFALLS: None specifically triggered beyond standard 8-bit mode (mf=true) 
// and DP=0 assumption for the field module.
// HELPERS: check_treasure_emu(snes), check_npcs_emu(snes)

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x02=1, 0x54=1, 0x5A=1, 0x5C=1, 0xB1=1, 0xEA=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Multiple possible output RAM locations via delegates)

// REVERSED_FUNCTION: field::CheckTreasureNPC ($98:4C)