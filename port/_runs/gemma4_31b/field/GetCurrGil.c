#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x0, DP=0
// This routine retrieves the current Gil (3 bytes) from the save-state RAM
// and stores it into the Direct Page scratch area ($30-$32).
static void GetCurrGil_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // The routine performs three 8-bit loads and stores.
    // Since A is 8-bit, each LDA loads a single byte.
    ram[0x30] = ram[0x16A0]; // lda $16a0 / sta $30
    ram[0x31] = ram[0x16A1]; // lda $16a1 / sta $31
    ram[0x32] = ram[0x16A2]; // lda $16a2 / sta $32
}

// PITFALLS: None. Simple byte-copy with 8-bit mode A.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x16A0=1, 0x16A1=1, 0x16A2=1
//   output_ram: 0x30=1, 0x31=1, 0x32=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetCurrGil ($ED:2D)