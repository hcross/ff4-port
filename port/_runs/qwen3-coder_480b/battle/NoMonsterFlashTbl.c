// NoMonsterFlashTbl is a data table, not a code routine.
// Translating as a static const byte array in C.
static const uint8_t NoMonsterFlashTbl[24] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x16,
    0x51, 0x0d, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00
};

// PITFALLS: none (data table, no execution)
// HELPERS: none (data only)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  n/a (data)
//   entry_flags: n/a (data)
REVERSED_FUNCTION: battle::NoMonsterFlashTbl ($FE:D4)