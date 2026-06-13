#include "snes/snes.h"

// This routine is not actually a function, but a data table containing 
// 16-bit word pairs. In the original ASM, it is defined as a series of 
// .word directives. Since the request is for a C translation of the "routine",
// and this is data used for palette animation (Sylvan Cave), it is 
// represented as a constant array of 16-bit values.
static const uint16_t SylvanCaveAnimPal_table[] = {
    0x26a0, 0x2280, 0x1e40, 0x2200, 0x15c0, 0x1a00, 0x1e40, 0x2280,
    0x19c0, 0x1580, 0x1140, 0x0d00, 0x08c0, 0x0d00, 0x1140, 0x1580,
    0x0ce0, 0x0cc0, 0x08a0, 0x0880, 0x0460, 0x0480, 0x08a0, 0x08c0,
    0x26a0, 0x2280, 0x1e40, 0x2200, 0x15c0, 0x1a00, 0x1e40, 0x2280
};

// PITFALLS: None. This is a data table, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::SylvanCaveAnimPal ($FC:06)