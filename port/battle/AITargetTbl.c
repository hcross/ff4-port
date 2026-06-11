// AITargetTbl is a jump table of 20 16-bit pointers to AI target selection routines.
// Each entry is a little-endian address in the range $BB:B6-$BB:DB.
// No execution occurs here — this is pure data. The harness uses it by reading
// 16-bit values at known offsets. Mode flags are irrelevant.
//
// Entry mode: N/A (data table)
// All accesses are 16-bit absolute reads from $7E:00BB+B6..DB.
//
// Logic:
//   ram[$00BB + 0xB6 + 2*i] = low byte of address
//   ram[$00BB + 0xB7 + 2*i] = high byte of address
//   for i = 0..19
//
// This is a static data table, not a function. The C representation is an array
// of 20 function pointers, each pointing to a 16-bit address in the $BBxx range.
// However, since this is a data block and not executable code, we model it as
// a static const array of uint16_t addresses.

// PITFALLS: none (data table, no execution)
// HELPERS: none (data table)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
REVERSED_FUNCTION: battle::AITargetTbl ($BB:B6)