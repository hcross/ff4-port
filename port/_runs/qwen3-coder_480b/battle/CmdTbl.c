// CmdTbl is a jump table of 16-bit pointers to command handlers.
// The table is indexed by command ID (0-36), with some entries set to 0 (unused).
// No execution logic here — this is pure data. The harness uses it via address lookup.
//
// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=$7E, DP=0
//   (inferred from battle module conventions and absence of mode directives)
//
// No inputs or outputs — this is a static table in ROM mirrored to RAM.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
static const uint16_t CmdTbl_c[37] = {
    0xB300, // Cmd_00
    0xB301, // Cmd_01
    0xB302, // Cmd_02
    0x0000, // unused
    0x0000, // unused
    0xB305, // Cmd_05
    0xB306, // Cmd_06
    0xB307, // Cmd_07
    0xB308, // Cmd_08
    0xB309, // Cmd_09
    0xB30A, // Cmd_0a
    0xB30B, // Cmd_0b
    0xB30C, // Cmd_0c
    0xB30D, // Cmd_0d
    0xB30E, // Cmd_0e
    0xB30F, // Cmd_0f
    0xB310, // Cmd_10
    0xB311, // Cmd_11
    0xB312, // Cmd_12
    0xB313, // Cmd_13
    0xB314, // Cmd_14
    0x0000, // unused
    0xB316, // Cmd_16
    0xB317, // Cmd_17
    0x0000, // unused
    0xB319, // Cmd_19
    0xB31A, // Cmd_1a
    0xB31B, // Cmd_1b
    0xB31C, // Cmd_1c
    0xB31D, // Cmd_1d
    0xB31E, // Cmd_1e
    0xB31F, // Cmd_1f
    0xB320, // Cmd_20
    0xB321, // Cmd_21
    0xB322, // Cmd_22
    0x0000, // unused
    0xB324  // Cmd_24
};

// PITFALLS: none (data-only)
// HELPERS: none
REVERSED_FUNCTION: battle::CmdTbl ($B3:006C)