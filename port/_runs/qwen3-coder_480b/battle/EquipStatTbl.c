// EquipStatTbl is a data table, not a function. It's included directly in the C source
// as a static const array. No execution logic is required.
//
// Table content:
//   Offsets: +3/+0, +5/+0, +10/+0, +15/+0, +5/-5, +10/-10, +15/-15, +5/-10
//   Stored as 8 pairs of signed 8-bit values (delta_atk, delta_def)

static const uint8_t EquipStatTbl[16] = {
    0x03, 0x00,  // +3/+0
    0x05, 0x00,  // +5/+0
    0x0a, 0x00,  // +10/+0
    0x0f, 0x00,  // +15/+0
    0x05, 0xfb,  // +5/-5
    0x0a, 0xf6,  // +10/-10
    0x0f, 0xf1,  // +15/-15
    0x05, 0xf6,  // +5/-10
};

// PITFALLS: None (data table, no execution)
// HELPERS: None (no calls)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
REVERSED_FUNCTION: battle::EquipStatTbl ($FE:AC)