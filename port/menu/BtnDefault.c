#include "snes/snes.h"

// This routine is not an executable function but a data table of 16-bit 
// default button values. In the context of the snesrev/ff4 project, 
// "translating" a data table means providing a C representation of that 
// memory range so the parity harness can validate reads from it.
// Since the ASM contains only .word directives and no instructions, 
// it is effectively a constant array stored at $FE:86.
static const uint16_t BTN_DEFAULT_TABLE[] = {
    0x8000, 0x4000, 0x8000, 0x0000, 0x0800, 0x0400, 0x0200, 0x0100,
    0x0080, 0x0040, 0x0000, 0x0010
};

// To maintain parity with a routine that would read this table,
// the emulator or the C-side reader accesses snes->ram relative 
// to the bank $FE. However, as a pure data block, this doesn't have 
// a "body" in the traditional sense. 
//
// If the harness treats this as a "function" to be emulated, it 
// will simply return the value at the PC.

// PITFALLS: None (Data table, no logic)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0, db=0xFE
//   entry_flags: auto
//   CUSTOM_SPIKE: yes (Data table validation)

// REVERSED_FUNCTION: menu::BtnDefault ($FE:86)