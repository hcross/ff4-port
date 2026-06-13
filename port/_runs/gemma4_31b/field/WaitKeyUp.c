#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x00, DP=0
// Logic: Polls hardware registers $02 and $03 (input state) 
// and blocks until both are zero (keys released).
static void WaitKeyUp_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // This is a blocking poll loop.
    // In a real SNES emulator, ram[0x02] and ram[0x03] map to 
    // the joypad read registers.
    while (ram[0x02] != 0 || ram[0x03] != 0) {
        // The ASM checks $02 first, then $03.
        // If either is non-zero, it branches back to @8dfc.
    }
}

// PITFALLS: None. Simple polling loop.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x02=1, 0x03=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::WaitKeyUp ($8D:FC)