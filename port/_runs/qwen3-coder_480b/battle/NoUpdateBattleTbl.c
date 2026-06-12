// NoUpdateBattleTbl is a jump table consisting of 16-bit offsets.
// It is not a routine to be executed, but rather a data structure.
// The table contains 12 entries of 2 bytes each, followed by a $ff byte.
// Since this is not executable code, no C translation is needed.
// The data can be represented as a static const array in C.

static const uint16_t NoUpdateBattleTbl[12] = {
    0x00eb, 0x00f5, 0x00f0, 0x00f6, 0x00fa, 0x01a9,
    0x00fe, 0x00f3, 0x01b3, 0x01b4, 0x01b7, 0x01b8
};

// This is followed by a 0xff byte in the original data, which is likely
// a terminator or marker. Since it's not part of the 16-bit entries,
// it's not included in the array above.

// PITFALLS: None (this is a data table, not executable code)
// HELPERS: None (this is a data table, not executable code)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none
// CUSTOM_SPIKE: yes (data table, not a function)
REVERSED_FUNCTION: battle::NoUpdateBattleTbl ($FD:0000)