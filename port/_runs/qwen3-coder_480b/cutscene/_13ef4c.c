#include "snes/snes.h"

// This function initializes some cutscene state and reads a byte from a table.
// It clears two bytes at $7D21 and $7D23, sets $7D24 to $1A, increments
// $7D26, uses that as an index into $EE00 table, and stores the result in $7D22.
static void _13ef4c_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    ram[0x7D21] = 0;                    // stz $7d21
    ram[0x7D23] = 0;                    // stz $7d23
    ram[0x7D24] = 0x1A;                 // lda #$1a / sta $7d24
    uint8_t index = ram[0x7D26];        // lda $7d26
    ram[0x7D26] = index + 1;            // inc $7d26
    uint8_t value = ram[0xEE00 + index]; // tax / lda $ee00,x
    ram[0x7D22] = value;                // sta $7d22
}

// PITFALLS: None of the common pitfalls apply directly since there are no
// flag-dependent branches, no mode changes, no multi-byte operations, and
// no subroutine calls.
// HELPERS: None required - no subroutine calls made
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x7d26=1
//   output_ram:  0x7d21=1, 0x7d22=1, 0x7d23=1, 0x7d24=1, 0x7d26=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13ef4c ($EF:4C)