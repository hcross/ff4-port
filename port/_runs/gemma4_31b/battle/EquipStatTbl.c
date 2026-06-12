/**
 * EquipStatTbl is a data table mapping equipment levels/types to stat modifiers.
 * Since it consists entirely of .byte directives with no executable code, 
 * the "translation" in the context of the parity harness is the representation 
 * of this data in the emulator's ROM/RAM space.
 * 
 * However, for a C native reimplementation, this is expressed as a 
 * constant lookup table.
 */
static const uint8_t EQUIP_STAT_TBL[] = {
    0x03, 0x00, // +3 / +0
    0x05, 0x00, // +5 / +0
    0x0A, 0x00, // +10 / +0
    0x0F, 0x00, // +15 / +0
    0x05, 0xFB, // +5 / -5
    0x0A, 0xF6, // +10 / -10
    0x0F, 0xF1, // +15 / -15
    0x05, 0xF6  // +5 / -10
};

// Note: As this is a data table and not a routine, it does not have a 
// functional C body. Callers accessing this address in ASM will 
// read these bytes relative to $FE:AC.

// PITFALLS: None (Data table)
// HELPERS: None

// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFE
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::EquipStatTbl ($FE:AC)