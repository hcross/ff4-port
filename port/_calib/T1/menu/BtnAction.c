#include "snes/snes.h"

// The routine BtnAction is a lookup table mapping button inputs 
// to specific action flags or IDs. 
// Table contents: 0x0000, 0x0080, 0x8000, 0x0040
static const uint16_t BTN_ACTION_TABLE[] = {
    0x0000, 0x0080, 0x8000, 0x0040
};

// Note: Since this is a data table and not a functional routine, 
// there is no CPU state (registers/flags) to manipulate.
// Any logic using this table will be found in the routines that 
// reference address $FE:7E.

// PITFALLS: None (Data table, not executable code)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFE
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes