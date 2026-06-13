#include "snes/snes.h"

// The routine _15bb18 is not executable code, but a lookup table of constants.
// Each entry consists of 4 bytes. Since it contains no instructions,
// the "translation" is a functional mapping of the data accessed at this address.
// In the context of the parity harness, this is handled as a data-read
// from the ROM/RAM mirror at $BB:18.
static void field_15bb18_c(Snes *snes) {
    // This routine is a data table. No logic to execute.
}

// PITFALLS: None. This is a data table, not an executable routine.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0, db=0xBB
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Data table, not a function)

// REVERSED_FUNCTION: field::_15bb18 ($BB:18)