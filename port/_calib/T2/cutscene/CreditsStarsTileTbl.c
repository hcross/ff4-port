#include "snes/snes.h"

// This routine is a data table encoded as code. It is not a functional routine
// but rather a lookup table of bytes used by other code.
// No execution occurs here; the label is referenced by other routines
// performing indexed reads into this range.

// PITFALLS: none (data-only)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::CreditsStarsTileTbl ($ED:C7)