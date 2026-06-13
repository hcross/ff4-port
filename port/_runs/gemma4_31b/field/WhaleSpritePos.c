#include "snes/snes.h"

// Entry mode: A/X/Y mode inherited from caller.
// This routine is not a functional block of code but a data table 
// embedded in the code segment, providing offset pairs for whale sprite positioning.
// Each pair consists of a (low, high) byte.
// Based on the ASM source, it's a sequence of 16 bytes forming 8 pairs.
// Since the "routine" consists only of .byte directives and no executable instructions,
// "translating" it into a C function that mimics its effect is not applicable.
// However, in the context of a native reimplementation, this is a lookup table.
// If this was called via a jump or used as a data source, the parity harness 
// expects the memory at $BD:97 to contain these specific bytes.

static void WhaleSpritePos_c(Snes *snes) {
    // This "function" is actually a data table. 
    // In a native C port, this is represented as a constant array.
    // No CPU state is modified.
    (void)snes;
}

// PITFALLS: None. This is a data table embedded in the binary, not executable logic.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: auto
// CUSTOM_SPIKE: yes (Data table, no functional state change)

// REVERSED_FUNCTION: field::WhaleSpritePos ($BD:97)