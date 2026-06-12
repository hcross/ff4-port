// NoWinAnimTbl is a lookup table of 9 16-bit values followed by a $ff terminator.
// It is accessed via indexed addressing in other routines.
// This is a data label, not a function. Emitting as C array.

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

static const uint16_t NoWinAnimTbl_c[9] = {
    0x00eb, 0x00f5, 0x00f0, 0x00fa, 0x01a9,
    0x00fe, 0x00f3, 0x01b4, 0x01b7
};

// PITFALLS: none (data table, no execution)
// HELPERS: none (data only)
REVERSED_FUNCTION: battle::NoWinAnimTbl ($FE:76)