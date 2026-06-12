// This routine is a script table of magic IDs to use in auto-battle.
// It is not a code routine, but rather data referenced by AutoBattle.
// The format is pairs of bytes: [target, magic_id], terminated by 0xFF.
// No execution logic is present here to translate.

// PITFALLS: None (data-only, no execution)
// HELPERS: None (no calls)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::AutoBattle_0004 ($FE:003B)