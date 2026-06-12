// This is not a function, but a data table. 
// The assembly source defines a sequence of bytes.
// In the C reimplementation, this is represented as a constant array.
// It is accessed via pointer arithmetic in the assembly.
static const uint8_t NoNameAttackTbl[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x12,
    0x00, 0x03, 0x10, 0x00, 0x00, 0x67, 0xFF, 0xFF
};

// PITFALLS: None. This is a static data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none
// CUSTOM_SPIKE: yes (This is a data table, not an executable routine)

REVERSED_FUNCTION: battle::NoNameAttackTbl ($FE:BC)