#include "snes/snes.h"

// This function contains only raw data bytes, not executable 65816 code.
// It represents a 32-entry table of 8-byte records, likely used for
// graphical or animation data in the cutscene module.
//
// Each 8-byte entry has the following structure (based on content):
//   [0-1] word: some kind of offset or identifier
//   [2-3] word: another value, possibly a frame or tile index
//   [4-5] word: often $FFFF, may be a flag or end marker
//   [6-7] word: another value, often constant like $92xx or $49xx
//
// Since this is a data table and not a function, no translation to C
// logic is required. It is included in the disassembly for reference
// and will be embedded in the C build as a static const array.

// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: cutscene::_13df57 ($DF:57)