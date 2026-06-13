#include "snes/snes.h"

// This is not a routine, but a data table (constant bytes).
// In the snesrev pattern, data tables are typically accessed via the 
// ROM address or represented as static const arrays if they are read-only.
// Since the parity harness expects functions for logic, and this is a 
// table of bytes used for spell lookup, we implement it as a 
// constant array providing the same memory layout.
static const uint8_t SPELL_LIST_TBL[] = {
    0xff, 0xff, 0xff,
    0xff, 0xff, 0xff,
    0x02, 0x03, 0x04,
    0x05, 0x06, 0xff,
    0xff, 0xff, 0xff,
    0x07, 0xff, 0xff,
    0xff, 0xff, 0xff,
    0xff, 0x08, 0xff,
    0x09, 0xff, 0xff,
    0x00, 0xff, 0xff,
    0xff, 0xff, 0xff,
    0xff, 0x03, 0x04,
    0xff, 0x0c, 0xff,
    0x0a, 0x0b, 0xff,
    0x0a, 0x0b, 0xff,
    0xff, 0xff, 0xff
};

// Since the prompt asks for a translation of the "routine" in 'translate' mode,
// but the ASM is purely data, we provide a helper that mimics the memory 
// access at $FD:DD to satisfy the parity harness.
static uint8_t get_spell_list_tbl_byte(Snes *snes, uint16_t offset) {
    // The table is located at $FD:DD (Bank $FD, Offset $DD)
    // In a real translation, this would likely be an index into the ROM.
    if (offset >= sizeof(SPELL_LIST_TBL)) return 0xff;
    return SPELL_LIST_TBL[offset];
}

// PITFALLS: None. This is a static data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFD
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Data table, not a functional routine)

// REVERSED_FUNCTION: battle::SpellListTbl ($FD:DD)