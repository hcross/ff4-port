#include "snes/snes.h"

// Clamp A to maximum 99. Entry: A = value (8-bit). Exit: A = min(A, 99).
static void Max99_c(Snes *snes) {
    uint8_t a = snes->cpu->a & 0xFF;
    if (a >= 99) {             // bcc not taken → carry set means A ≥ 99 (Pitfall 3)
        snes->cpu->a = 99;     // lda #99
    }
}

// PITFALLS: 3 (CMP/BCC: bcc branches when A < 99 unsigned, so we enter
// the lda path when A >= 99)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  (none)
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: battle::Max99 ($9E:20)