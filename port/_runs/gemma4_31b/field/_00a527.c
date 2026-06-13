#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$A5, DP=0
// Purpose: Clamps the value at RAM $79 to a maximum of 15 (0x0F).
// If the value is less than 16 (0x10), it is written to hINIDISP.
// hINIDISP is defined as 0x38B2 in the field module.
static void _00a527_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    uint8_t val = ram[0x79];
    
    // cmp #$10 / bcs @a530
    // Pitfall 3: bcs branches when A >= 0x10. 
    // We enter the block only if A < 0x10.
    if (val < 0x10) {
        ram[0x38B2] = val; // sta hINIDISP
    }
}

// PITFALLS: 3 (CMP/BCS inversion)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x79=1
//   output_ram: 0x38B2=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xA5
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00a527 ($A5:27)