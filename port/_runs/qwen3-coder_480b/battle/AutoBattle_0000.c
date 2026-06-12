// This routine is a data script, not a code routine. It represents
// an auto-battle action script that uses a fire bomb (item $01C0) and ends.
// There is no execution logic to translate; the parity harness will
// validate that the data at $FE:2D matches the expected script format.
//
// Script format (as inferred from usage in battle system):
//   - $01,$C0 : use item 0x01C0 (fire bomb)
//   - $FF     : end of script marker

// PITFALLS: None (data-only, no execution)
// HELPERS: None (no calls)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::AutoBattle_0000 ($FE:2D)