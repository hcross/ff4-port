// This routine is a data script, not a code routine. It contains a sequence
// of bytes interpreted by the auto-battle script engine.
// No execution logic to translate — the script means:
//   use item 0x01C2 (lit-bolt), then end script (0xFF).
//
// Since this is not executable code, no translation to C is needed.
// The script is interpreted by the auto-battle engine at runtime.

// CONTRACT:
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AutoBattle_0001 ($FE:30)