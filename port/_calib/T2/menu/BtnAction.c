#include "snes/snes.h"

// BtnAction is a jump table located in ROM at $FE:007E.
// It is not a routine to be executed, but a data structure.
// The table contains 4 16-bit pointers: [$0000, $0080, $8000, $0040].
// This is likely used by code that reads from this table to get a pointer
// based on an index. Since it's data, not code, there's no C function to
// implement. Instead, we represent it as a comment in the ROM/data area.

// No C function body is needed. This is pure data.

// PITFALLS: none (data, not code)
// HELPERS: none
// CONTRACT: none (data, not code)
// REVERSED_FUNCTION: menu::BtnAction ($FE:007E)