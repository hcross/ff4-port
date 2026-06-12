// This is not a routine, but a data table containing a list of 
// 16-bit offsets/pointers, terminated by a 0xFF byte.
// Since it is data, the "translation" is a constant array 
// providing the same values at the same relative addresses.
static const uint16_t NO_FANFARE_TBL[] = {
    0x00DC, 0x00DD, 0x00E1, 0x00E7, 0x01A7, 0x01AF, 0x01B6
};

// To maintain parity with the original ROM layout when the emulator 
// or C code reads from $FE:67, the data is accessed as:
// snes->ram[0xFE67] etc. (though usually mapped to ROM/Bank)
// Note: The .byte $ff is the sentinel value.

// PITFALLS: None. This is a static data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none

REVERSED_FUNCTION: battle::NoFanfareTbl ($FE:67)