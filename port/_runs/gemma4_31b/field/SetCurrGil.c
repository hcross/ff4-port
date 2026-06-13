#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), DB=0xED, DP=0
// This routine copies 3 bytes from the direct page ($30, $31, $32) 
// to WRAM ($08F8, $08F9, $08FA), effectively updating the 
// current gold (Gil) value in the game state.
static void SetCurrGil_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // The routine uses absolute addressing for DP offsets $30-$32
    // and absolute addressing for target WRAM $08F8-$08FA.
    // Since DP is 0, ram[0x30] is the direct mapping.
    ram[0x08F8] = ram[0x30]; // lda $30 / sta $08f8
    ram[0x08F9] = ram[0x31]; // lda $31 / sta $08f9
    ram[0x08FA] = ram[0x32]; // lda $32 / sta $08fa
}

// PITFALLS: None. This is a straightforward byte-copy routine.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x30=1, 0x31=1, 0x32=1
//   output_ram:  0x08F8=1, 0x08F9=1, 0x08FA=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xED
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::SetCurrGil ($ED:001D)