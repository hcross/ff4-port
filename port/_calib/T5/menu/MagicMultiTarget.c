#include "snes/snes.h"

// MagicMultiTarget is a 14-byte lookup table in ROM at $FF:F2.
// The original "routine" is just data; callers do `lda MagicMultiTarget,x`
// to read the multi-target flag for a magic spell (index 0–13).
// This C function replicates that lookup: given an 8-bit index in X,
// it returns the corresponding byte in A and sets Z/N accordingly.
static void MagicMultiTarget_c(Snes *snes) {
    static const uint8_t table[14] = {
        1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1
    };
    uint8_t index = (uint8_t)snes->cpu->x;   // X is 8-bit (xf=true)
    uint8_t result = (index < 14) ? table[index] : 0; // out-of-range → 0
    snes->cpu->a = result;                   // 8-bit A (mf=true)
    snes->cpu->z = (result == 0);
    snes->cpu->n = (result & 0x80) != 0;
}

// PITFALLS: 2 (caller expects Z/N to reflect the loaded value; we set them
//          explicitly to match the effect of `lda table,x`).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  x=8
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0xFF
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes  (original is a data table, not a subroutine;
//                        harness must compare table bytes or simulate
//                        `lda table,x` behaviour)
// REVERSED_FUNCTION: menu::MagicMultiTarget ($FF:F2)