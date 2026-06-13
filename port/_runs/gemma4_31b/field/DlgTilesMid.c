#include "snes/snes.h"

// This routine is not a functional piece of code but a data table
// containing tile indices and widths for dialogue box rendering.
// The ASM provided consists solely of .byte directives.
// In a C native reimplementation, this is represented as a static array.
static const uint8_t DlgTilesMid_data[] = {
    0x00, 0x20, 0x1B, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20,
    0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20,
    0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20,
    0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1C, 0x20, 0x1D, 0x20, 0x00, 0x20
};

// Since the original "routine" was actually data, the parity harness 
// validates this by checking that reads to $F7:16 return the bytes above.
// We provide a getter to maintain the "function" interface if required by the harness.
static uint8_t DlgTilesMid_read(Snes *snes, uint32_t offset) {
    return DlgTilesMid_data[offset];
}

// PITFALLS: None (Data table, no logic)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DlgTilesMid ($F7:16)