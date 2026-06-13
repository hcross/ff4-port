#include "snes/snes.h"

// CmdTbl is a ROM data table (37 entries of 16-bit command handler offsets
// within bank $B3). It is not an executable routine — the asm source
// consists entirely of `.addr` directives. This C function is a no-op
// placeholder; the actual table bytes reside in the ROM image and are
// referenced by other translated routines via their ROM address.
static void CmdTbl_c(Snes *snes) {
    (void)snes;
    // No operation — data table only.
}

// PITFALLS: none (no instructions, no branches, no mode dependencies)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  n/a (data table, not executed)
//   entry_flags: n/a
// CUSTOM_SPIKE: yes  (data table — parity harness should skip)
// REVERSED_FUNCTION: battle::CmdTbl ($B3:6C)