// This routine is a data table interpreted by battle AI script engine.
// It is not a callable function and should not be translated to C.
// The data represents scripted actions for an auto-battle AI:
//   - Use magic "meteo" (0x2F)
//   - Use magic "dispel" (0x0C)
//   - End script
// Followed by an alternate script for "golbez":
//   - Use magic "meteo" (0x2F)
//   - Use item "crystal" (0xC8)
//   - End script
//
// No executable code exists here to translate. This is pure data.
//
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::AutoBattle_0007 ($FE:5D)