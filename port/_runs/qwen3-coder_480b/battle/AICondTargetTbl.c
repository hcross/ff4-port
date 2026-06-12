// AICondTargetTbl is a jump table of 53 16-bit pointers to AI condition handlers.
// Each entry is a 2-byte little-endian address, forming an array of function pointers.
// The table is indexed by AI condition ID (0-52), and read by the AI decision engine.
//
// No execution logic here — this is pure data. The C translation mirrors the
// 65816 assembly layout as a flat array of 16-bit addresses in WRAM.
//
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
// CUSTOM_SPIKE: yes

// This is a data table, not a function. The parity harness handles this as a static data blob in WRAM.
// In C, this would be represented as:
//   uint16_t AICondTargetTbl[53] = { ... };
// However, since the task is to translate a *routine*, and this is data,
// the harness will skip this as CUSTOM_SPIKE.

REVERSED_FUNCTION: battle::AICondTargetTbl ($C1:000C)