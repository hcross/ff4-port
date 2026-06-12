// AITargetTbl is a jump table of 20 16-bit pointers to AI target selection routines.
// Each entry is a 16-bit address, stored in little-endian format.
// No execution logic — this is pure data. Provided as C array for emulator lookup.

static const uint16_t AITargetTbl[20] = {
    0xB616, 0xB617, 0xB618, 0xB619, 0xB61A, 0xB61B, 0xB61C, 0xB61D,
    0xB61E, 0xB61F, 0xB620, 0xB621, 0xB622, 0xB623, 0xB624, 0xB625,
    0xB626, 0xB627, 0xB628, 0xB629
};

// PITFALLS: none (data-only, no execution)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=auto, xf=auto, dp=auto, db=auto
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITargetTbl ($BB:B6)