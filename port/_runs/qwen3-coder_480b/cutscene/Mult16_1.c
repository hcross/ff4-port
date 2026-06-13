#include "snes/snes.h"

// 16-bit multiplication: result = $0E * $10, stored in $12 (lo) and $14 (hi)
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Uses 16-bit A internally (longa), but restores 8-bit before return
static void Mult16_1_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t multiplicand = read16(ram, 0x0E);  // $0e
    uint16_t multiplier   = read16(ram, 0x10);  // $10
    uint32_t result = (uint32_t)multiplicand * multiplier;
    write16(ram, 0x12, (uint16_t)(result & 0xFFFF));      // $12 = lo
    write16(ram, 0x14, (uint16_t)((result >> 16) & 0xFFFF)); // $14 = hi
}

// PITFALLS: 1 (DB=$7E required for correct absolute addressing),
//           6 (mode A 16-bit during math, restored to 8-bit on exit),
//           8 (inherited 8-bit A on entry, 16-bit X)
// HELPERS: read16/write16 for 16-bit little-endian access
// CONTRACT:
//   inputs_ram:  0x0E=2, 0x10=2
//   output_ram:  0x12=2, 0x14=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::Mult16_1 ($E5:12)