#include "snes/snes.h"

// AICondTbl is a jump table of function pointers to AI condition routines.
// Each entry is a 16-bit address (little-endian) pointing to an AICond_XX routine.
// The table is indexed by a condition ID (0-11) to select which condition to evaluate.
// No execution logic here — just a data table used by the AI decision engine.

// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=$7E, DP=0
// (inferred from battle module conventions and lack of mode-setting ops)
//
// No register inputs or outputs. This is a static table read by callers
// via indirect JSR or indexed load. RAM layout must match exactly:
//   ram[$C0F4] = low(AICond_00), ram[$C0F5] = high(AICond_00)
//   ram[$C0F6] = low(AICond_01), ram[$C0F7] = high(AICond_01)
//   ... and so on up to AICond_0b

// PITFALLS: 1 (DB=$7E required for correct absolute addressing)
//           8 (mode A/X heritage — table assumes 16-bit pointers)
// HELPERS: none (this is a data table, not a routine)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::AICondTbl ($C0:F4)