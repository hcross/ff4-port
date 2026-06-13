#include "snes/snes.h"

// This routine is a data table rather than an executable function.
// In the context of a C native reimplementation, this is translated 
// as a constant array lookup.
// The routine's "translation" provides the value at the given index.

static uint8_t NPCJumpYTbl_c(Snes *snes, uint8_t index) {
    static const uint8_t jump_y_tbl[32] = {
        0x00, 0x03, 0x06, 0x08, 0x0B, 0x0D, 0x0F, 0x11, 
        0x13, 0x14, 0x15, 0x17, 0x17, 0x18, 0x19, 0x19,
        0x19, 0x19, 0x19, 0x18, 0x17, 0x17, 0x15, 0x14, 
        0x13, 0x11, 0x0F, 0x0D, 0x0B, 0x08, 0x06, 0x03
    };

    if (index >= 32) return 0; // Bounds safety for C implementation
    return jump_y_tbl[index];
}

// PITFALLS: None (Data table translation)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFC
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (This is a data table lookup, not a routine with side effects)

// REVERSED_FUNCTION: field::NPCJumpYTbl ($FC:C6)