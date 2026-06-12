// NoNameAttackTbl is a data table, not a routine. It's 24 bytes long
// and resides at $7E:FEBC. It is used by other routines via indexed access.
// No executable code exists here, so no translation is needed.
//
// Data layout:
//   16 bytes of $00
//   2 bytes of $12
//   1 byte of $00
//   1 byte of $03
//   1 byte of $10
//   1 byte of $00
//   1 byte of $00
//   1 byte of $67
//   2 bytes of $FF

// PITFALLS: none (data table, not code)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::NoNameAttackTbl ($FE:BC)