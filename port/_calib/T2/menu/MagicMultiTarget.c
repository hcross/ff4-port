#include "snes/snes.h"

// This routine contains no executable code, only a data table.
// The label MagicMultiTarget points to a 14-byte array of constants.
// No emulation or logic is required — this is pure data.
//
// Entry mode: N/A (data only)
// All bytes are literals, no register or flag dependencies.
static void MagicMultiTarget_c(Snes *snes) {
    // Data table @ $FF:F2 (14 bytes)
    static const uint8_t data[14] = {1,1,1,1,0,0,0,1,0,0,1,1,1,1};
    // No operation needed; this is a data reference point.
    // If accessed, the caller reads snes->ram[0xFFF2 + offset].
    (void)snes; // suppress unused parameter warning
}

// PITFALLS: none (data-only)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  N/A
//   entry_flags: N/A
// CUSTOM_SPIKE: yes (data-only, no execution)
// REVERSED_FUNCTION: menu::MagicMultiTarget ($FF:F2)