#include "snes/snes.h"

// This routine is a lookup table access pattern. The table is embedded
// directly in the function body as a 32-byte array. The input index
// (presumably in A or X) selects a byte from the table and returns it.
//
// Entry assumption: A or X holds the index (0-31), other register state
// is irrelevant. No flags or memory inputs. Output is the table value.
static uint8_t _13e4f2_c(Snes *snes, uint8_t index) {
    // Embedded lookup table from $E4:F2
    static const uint8_t table[32] = {
        13,10,8,7,5,4,4,3,2,2,1,1,1,0,0,0,
        0,0,0,1,1,1,2,2,3,4,4,5,7,8,10,13
    };
    // Clamp index to valid range to prevent out-of-bounds access
    if (index >= 32) index = 0;
    return table[index];
}

// PITFALLS: None (pure data table, no execution logic)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13e4f2 ($E4:F2)