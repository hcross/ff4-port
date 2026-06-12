// CmdTargetTbl is a 26-byte lookup table indexed by command ID.
// It maps each command to a targeting mode used in battle.
// The table is read directly by other routines via ram[0x7E00 + offset].
//
// No execution flow — this is pure data embedded in the code section.
// Marked as a function for linker/layout purposes only.

// PITFALLS: none (data-only)
// HELPERS: none (no code)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none

REVERSED_FUNCTION: battle::CmdTargetTbl ($FD:C3)