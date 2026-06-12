// AICondTbl is a jump table of 12 16-bit pointers to AI condition routines.
// Each entry is a 16-bit address in the format $xxxx.
// The table is indexed by a condition ID (0-11) to select a routine.
// No execution logic — this is pure data. C translation mirrors the .addr list.
//
// Entry mode: Not applicable (data table, not a routine).
// No emulation helpers required — this is not a callable routine.
// CONTRACT:
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
static const uint16_t AICondTbl_c[12] = {
    0x00, // AICond_00
    0x01, // AICond_01
    0x02, // AICond_02
    0x03, // AICond_03
    0x04, // AICond_04
    0x05, // AICond_05
    0x06, // AICond_06
    0x07, // AICond_07
    0x08, // AICond_08
    0x09, // AICond_09
    0x0A, // AICond_0a
    0x0B  // AICond_0b
};

// PITFALLS: none (data table)
// HELPERS: none (data table)
// CONTRACT:
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICondTbl ($C0:F4)