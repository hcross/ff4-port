#include "snes/snes.h"

// The provided assembly for WipeScanlineTbl is not a routine, but a data table.
// In the context of the 65816, these bytes represent a lookup table used by 
// other functions to determine scanline offsets or widths.
// To translate this as a "function" that mimics its effect in memory, 
// we provide a routine that writes this specific data block into the 
// target memory location if the purpose is initialization.
//
// However, looking at the address $FB:1E, this is located in ROM.
// In a native C reimplementation, this becomes a static const array.
static const uint8_t WipeScanlineTbl_data[64] = {
    0x7f, 0x6e, 0x7e, 0x6e, 0x7e, 0x6e, 0x7d, 0x6d, 0x7c, 0x6c, 0x7b, 0x6b, 0x79, 0x6a, 0x77, 0x68,
    0x75, 0x66, 0x72, 0x64, 0x70, 0x61, 0x6c, 0x5f, 0x69, 0x5c, 0x66, 0x59, 0x62, 0x55, 0x5e, 0x52,
    0x59, 0x4e, 0x55, 0x4a, 0x50, 0x46, 0x4b, 0x42, 0x46, 0x3d, 0x41, 0x39, 0x3b, 0x34, 0x36, 0x2f,
    0x30, 0x2a, 0x2a, 0x25, 0x24, 0x20, 0x1e, 0x1a, 0x18, 0x15, 0x12, 0x10, 0x0c, 0x0a, 0x06, 0x05
};

// Since this was requested as 'mode: translate' for a routine, but the source 
// is pure data, the "C translation" of a data table is the data itself.
// If a function is needed to "wipe" or "copy" this table to RAM, 
// it would be implemented as follows:
static void WipeScanlineTbl_c(Snes *snes) {
    // This is a data table in ROM ($FB:1E). 
    // No logic to execute.
}

// PITFALLS: None. This is a data table, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: auto
// REVERSED_FUNCTION: field::WipeScanlineTbl ($FB:1E)