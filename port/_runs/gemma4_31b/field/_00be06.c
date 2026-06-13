#include "snes/snes.h"

// This "routine" is actually a data table containing a sequence of bytes.
// It does not contain executable 65816 instructions.
// The values are: 02, 02, 03, 00, 01, 02, 02, 02
static const uint8_t data_00be06[] = {
    0x02, 0x02, 0x03, 0x00,
    0x01, 0x02, 0x02, 0x02
};

// If this is being accessed by other translated C functions, 
// they should refer to snes->rom[0xBE06] or a similar offset.