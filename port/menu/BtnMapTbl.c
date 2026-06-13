#include "snes/snes.h"

// The BtnMapTbl is a lookup table used to map input buttons to 
// specific internal action codes.
static const uint8_t BTN_MAP_TBL[] = {
    0x28, 0x18, 0x2A, 0x1A, 0x1C
};

// To maintain parity with the memory-mapped layout used by the 
// emulator/harness, this table is located at $FE:79.
// In a fully native C port, this is accessed via the array above.