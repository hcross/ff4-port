// This function contains no executable code, only data.
// It represents two battle AI scripts (one for Cecil, one for Golbez)
// stored as byte sequences in ROM. The scripts are interpreted by
// the battle system at runtime.
//
// Script format (inferred):
// - Pairs of bytes: $00, <spell_id> → use magic spell
// - Pair: $D0, $00 → use command "twin"
// - $FF → end of script
//
// No emulation or RAM access required — pure data.

// PITFALLS: none (data only, no execution)
// HELPERS: none (no calls)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::AutoBattle_0006 ($FE:4B)