#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Initializes global game state for a new game, including clearing 
// specific WRAM markers, copying NPC/Event switches from ROM to RAM,
// and setting initial flags for game progress.
static void InitNewGame_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Clear specific state offsets
    ram[0x1700] = 0;
    ram[0x1701] = 0;
    ram[0x1703] = 0;
    ram[0x170F] = 0;
    ram[0x1712] = 0;
    ram[0x1718] = 0;
    ram[0x171C] = 0;
    ram[0x1720] = 0;
    ram[0x1724] = 0;
    ram[0x1728] = 0;
    ram[0x172B] = 0;
    ram[0x171B] = 0;
    ram[0x171F] = 0;
    ram[0x1723] = 0;
    ram[0x1727] = 0;
    ram[0x172C] = 0;

    // Initial NPC Switches: Copy 0x40 bytes from ROM to RAM $12E0
    // Note: ROM access is handled via the snes instance mapping for f:InitNPCSwitch
    for (uint16_t x = 0; x < 0x40; x++) {
        ram[0x12E0 + x] = snes->rom[0xInitNPCSwitch + x]; 
    }

    // Initial Event Switches: Copy 0x20 bytes from ROM to RAM $1280
    for (uint16_t x = 0; x < 0x20; x++) {
        ram[0x1280 + x] = snes->rom[0xInitEventSwitch + x];
    }

    // Clear event flags in RAM $12A0 (0x20 bytes)
    for (uint16_t x = 0; x < 0x20; x++) {
        ram[0x12A0 + x] = 0;
    }

    // Set specific game start markers
    ram[0x149C] = 0xFE;
    ram[0x149E] = 0xFF;
    ram[0x149D] = 1;
    ram[0x149F] = 1;
}

// PITFALLS: 1 (DB=$7E assumed for WRAM access)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x12E0=64, 0x1280=32, 0x12A0=32, 0x149C=1, 0x149D=1, 0x149E=1, 0x149F=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::InitNewGame ($82:90)