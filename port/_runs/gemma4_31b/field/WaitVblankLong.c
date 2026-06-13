#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// Logic:
//   1. Sets ram[0x7D] to 1.
//   2. Polls ram[0x7D] until it becomes 0 (Wait for VBlank interrupt to clear the flag).
//   3. Calls the VBlank synchronization/processing routine _15c23d.
//   4. Increments ram[0x7D] (effectively setting it back to 1).
static void WaitVblankLong_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x7D] = 1;                    // lda #1 / sta $7d

    while (ram[0x7D] != 0) {          // lda $7d / bne @9131
        // Busy wait for hardware/interrupt to clear ram[0x7D]
    }

    _15c23d_emu(snes);                // jsl _15c23d (delegated)
    ram[0x7D]++;                      // inc $7d
}

// PITFALLS: None relevant for this routine.
// HELPERS: _15c23d_emu(snes) — delegates routine @$15:C23D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x007D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::WaitVblankLong ($91:2D)