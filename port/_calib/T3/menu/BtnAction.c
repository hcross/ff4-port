#include "snes/snes.h"

// Button action lookup table (4 entries, 16-bit each)
// Referenced by menu code as BtnAction[idx] to get action flags.
static const uint16_t BtnAction[4] = {
    0x0000,
    0x0080,
    0x8000,
    0x0040,
};