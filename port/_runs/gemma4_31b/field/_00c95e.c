#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0, DP=0
// Purpose: Fills two sprite palette tables ($0E1B and $0E3B) with white ($FF)
// for 16 entries each.
static void _00c95e_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    for (uint16_t x = 0; x < 0x10; x++) { // ldx #0 / inx / cpx #$0010 / bne
        ram[0x0E1B + x] = 0xFF;          // sta $0e1b,x
        ram[0x0E3B + x] = 0xFF;          // sta $0e3b,x
    }
}

// PITFALLS: None. Simple loop with absolute indexed addressing.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0E1B=1, 0x0E3B=1 (ranges 0x10 bytes)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::_00c95e ($C9:5E)