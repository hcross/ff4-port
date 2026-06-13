#include "snes/snes.h"

// This routine is not a functional block of code, but a data table of 16-bit 
// color palette values used for Tower animations. 
// It consists of a sequence of 16 words (32 bytes) forming a symmetric 
// fade-in/fade-out sequence of palette indices/values.
static const uint16_t TowerAnimBluePal_Data[] = {
    0x7FFF, 0x7B91, 0x76C0, 0x6620, 0x5940, 0x44A0, 0x2800, 0x0000,
    0x0000, 0x2800, 0x44A0, 0x5940, 0x6620, 0x76C0, 0x7B91, 0x7FFF
};

// Since the routine is purely data, the "translation" into C involves 
// exposing this data table for use by the functions that reference it.
// If this is called as a function in the asm, it would likely result 
// in the CPU executing the data as opcodes, but given the name 
// and content, it is a lookup table.

// PITFALLS: None. This is a data definition.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::TowerAnimBluePal ($F8:26)