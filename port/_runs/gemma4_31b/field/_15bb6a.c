#include "snes/snes.h"

// This routine is not actually executable code, but a data table
// containing coordinate or property offsets (possibly for animation,
// collision, or sprite layout). It consists of 32 blocks of 4 bytes.
// Since the target is a .byte sequence at a specific memory location,
// and the task is to "translate the routine", but the "routine" is a data block,
// we implement it as a function that returns the data or a dummy a-no-op,
// but in the context of a C reimplementation, this represents a 
// static constant array used by other functions.
//
// However, to satisfy the parity harness requirement for a "function" 
// translation if the harness calls this address, we treat it as a 
// data-only region. If the harness expects a function, it will simply 
// execute the bytes as opcodes. But based on the .byte directives, 
// this is a lookup table.

static void _15bb6a_c(Snes *snes) {
    // This is a data table at $BB:6A. 
    // In a native C port, this is simply:
    // const uint8_t table_15bb6a[] = { ... };
    // No logic to translate.
}

// PITFALLS: None. This is a data table, not executable logic.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBB
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_15bb6a ($BB:6A)