// ActionTbl is a jump table of 4 function pointers (16-bit addresses)
// stored in ROM at $8921-$8928. Each entry is a .addr (2 bytes).
// This routine is data-only; no execution occurs here.
//
// Entry mode: N/A (this is not a callable routine, just data)
// All access through ROM reads at fixed addresses.
//
//   GetCharAttack    = $8921
//   GetMonsterAttack = $8923
//   DoAttack         = $8925
//   DoTimerEffect    = $8927

// PITFALLS: none (data-only)
// HELPERS: none (data-only)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::ActionTbl ($89:21)