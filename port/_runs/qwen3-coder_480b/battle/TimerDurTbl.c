// Table of function pointers (addresses), indexed by timer type.
// Each entry is a 16-bit little-endian address.
// No execution logic — this is a data table.
//
// CONTRACT:
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::TimerDurTbl ($9F:FB)