// This routine is a data script, not a code routine. It contains no executable
// instructions, only static data bytes interpreted by the auto-battle system.
// The label AutoBattle_0002 points to a script sequence:
//   @fe33: .byte $c0,$00  → use command: fight (command_id = 0x00, target = 0xc0)
//   @fe35: .byte $ff      → end of script marker
//
// Since this is not a function but a data array, no C translation is needed.
// The data is already embedded in the ROM and accessed by the auto-battle engine.

// PITFALLS: None (not a code routine)
// HELPERS: None (no calls)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AutoBattle_0002 ($FE:33)