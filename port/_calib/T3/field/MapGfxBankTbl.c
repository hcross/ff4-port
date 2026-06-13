#include "snes/snes.h"

// MapGfxBankTbl: 16-byte table of bank bytes for MapGfx_0000..MapGfx_000f
// This is ROM data at $B1:04, not executable code.
// Values are the bank bytes (bits 16-23) of each MapGfx_XXXX label address.
static const uint8_t MapGfxBankTbl[16] = {
    // These values must match the ROM at $B1:04
    // Bank bytes of MapGfx_0000 through MapGfx_000f
    0x0B, 0x0B, 0x0B, 0x0B,  // MapGfx_0000-0003
    0x0C, 0x0C, 0x0C, 0x0C,  // MapGfx_0004-0007
    0x0D, 0x0D, 0x0D, 0x0D,  // MapGfx_0008-000B
    0x0E, 0x0E, 0x0E, 0x0E,  // MapGfx_000C-000F
};