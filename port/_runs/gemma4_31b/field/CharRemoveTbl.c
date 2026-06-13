#include "snes/snes.h"

// This routine is not a function, but a lookup table (data) 
// used by the field module to determine replacement characters 
// or removal flags for specific bytes.
// Since it is pure data, the "translation" provides a constant array 
// mapping the bytes defined in the ASM source.
static const uint8_t CharRemoveTbl[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x81, 0x09, 0x81, 0x83, 0x0a, 0x80, 0x82,
    0x84, 0x0b, 0x0c, 0x80, 0x0d
};

// PITFALLS: None (Data table)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (This is a data table, parity is verified via ROM checksum/read)

// REVERSED_FUNCTION: field::CharRemoveTbl ($E9:22)