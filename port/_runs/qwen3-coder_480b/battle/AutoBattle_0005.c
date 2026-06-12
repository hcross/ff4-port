// This routine is a data table, not a code routine. It contains script bytes
// for an auto-battle action: fight command followed by flame magic, then end.
// No execution logic is present — the caller reads this as a byte sequence.
//
// Entry mode: N/A (not a code routine)
// No register or flag state required.
// No emulation or helpers needed.
//
// Data layout:
//   @fe46:  c0 00        use command: fight
//   @fe48:  00 42        use magic: flame
//   @fe4a:  ff           end of script

// PITFALLS: none (data, not code)
// HELPERS: none (data, not code)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=auto, xf=auto, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::AutoBattle_0005 ($FE:46)