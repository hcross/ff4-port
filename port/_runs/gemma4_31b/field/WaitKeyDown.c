#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x8E, DP=0
// This routine polls for any key press (defined by memory $02 and $03).
// It loops until either $02 is non-zero or $03 is non-zero.
static void WaitKeyDown_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    while (1) {
        if (ram[0x02] != 0) { // lda $02 / bne @8e0d
            break;
        }
        if (ram[0x03] == 0) { // lda $03 / beq @8e05
            continue;
        }
        break; // fell through to @8e0d
    }
}

// PITFALLS: None applicable. Simple polling loop.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x02=1, 0x03=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x8E
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::WaitKeyDown ($8E:05)