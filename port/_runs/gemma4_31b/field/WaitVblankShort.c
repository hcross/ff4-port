#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine synchronizes with the VBlank by polling a hardware/RAM flag,
// calls a system function (likely a VBlank-related hook), and increments
// a counter at 0x7D.
static void WaitVblankShort_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x7E] = 1; // lda #1 / sta $7e
    
    // Poll loop: Wait until ram[0x7E] becomes 0
    while (ram[0x7E] != 0) { // lda $7e / bne @9140
        // Tight loop matching ASM behavior
    }

    _15c23d_emu(snes); // jsl _15c23d (delegated)
    ram[0x7D]++;       // inc $7d
}

// PITFALLS: None relevant. Straightforward polling loop and increment.
// HELPERS: _15c23d_emu(snes) — delegates routine at $15:C23D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x7D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::WaitVblankShort ($91:3C)