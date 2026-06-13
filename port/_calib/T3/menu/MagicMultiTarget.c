#include "snes/snes.h"

// Lookup table: 14 bytes indicating multi-target eligibility per magic spell type.
// Accessed by index from caller code; no executable logic.

static const uint8_t MagicMultiTarget[14] = {
    1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1
};

// PITFALLS: (none — pure data, no code)
// HELPERS: (none)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  n/a
//   entry_flags: n/a
// CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: menu::MagicMultiTarget ($FF:F2)