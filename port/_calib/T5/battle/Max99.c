#include "snes/snes.h"

// Clamp the low byte of A to a maximum of 99, preserving the hidden
// upper byte (B).  If A_low >= 99, replace it with 99; otherwise leave
// A unchanged.
static void Max99_c(Snes *snes) {
    uint16_t a_full = snes->cpu->a;          // full 16-bit C register
    uint8_t  a_low  = a_full & 0xFF;        // low byte (8-bit A)
    if (a_low >= 99) {                      // cmp #99 / bcc (inverted)
        a_low = 99;                         // lda #99
    }
    snes->cpu->a = (a_full & 0xFF00) | a_low; // preserve B (Pitfall 9)
}

// PITFALLS: 9 (hidden upper byte B must be preserved across an 8-bit
//          `lda #99`; setting `cpu->a = 99` would zero B and break parity)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none (output is in A)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto  (cmp #99 sets them internally)
// REVERSED_FUNCTION: battle::Max99 ($9E:20)