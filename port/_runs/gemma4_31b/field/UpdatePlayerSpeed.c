#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$83, DP=0
// Logic:
//   Reads the vehicle ID from RAM $1704, uses it as an index into 
//   the PlayerSpeedTbl (located in ROM), and writes the resulting 
//   movement speed byte to RAM $AC.
static void UpdatePlayerSpeed_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $1704 / tax
    uint8_t vehicle_id = ram[0x1704];

    // lda PlayerSpeedTbl,x
    // In the parity harness, ROM data is accessed via the 
    // snes_read_rom helper to avoid direct struct member access 
    // which may vary between emulator versions.
    uint8_t speed = snes_read_rom(snes, PlayerSpeedTbl + vehicle_id);

    // sta $ac
    ram[0xAC] = speed;
}

// PITFALLS: None. Routine is a simple load-index-store.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1704=1
//   output_ram:  0xAC=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x83
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::UpdatePlayerSpeed ($83:02)