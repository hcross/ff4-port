#include "snes/snes.h"

// field::_00bfe3 ($BF:E3)
// Reads two signed bytes from $0C and $0E. If either is negative, returns 0.
// Otherwise, uses them as a 16-bit index (low=$0C, high=$0E) into a table at
// $7F:5C71, shifts the fetched byte left by 1 (16-bit), then uses the low
// byte of the shifted result as an index into a table at $7E:0EDB, ANDs the
// fetched byte with 0x83, and returns that value.
//
// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DP=0, DB=$7E.
// The routine preserves X (phx/plx) — we leave snes->cpu->x untouched.
static uint8_t _00bfe3(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t val0c = ram[0x0C];
    if (val0c & 0x80) {          // bmi @c008 — negative → return 0
        return 0;
    }
    uint8_t val0e = ram[0x0E];
    if (val0e & 0x80) {          // bmi @c008
        return 0;
    }

    // Both non-negative: store to $3D/$3E (temporary workspace)
    ram[0x3D] = val0c;
    ram[0x3E] = val0e;

    // ldx $3D (16-bit) → index = (val0e << 8) | val0c
    uint16_t index1 = ((uint16_t)val0e << 8) | val0c;

    // lda $7F5C71,x — bank $7F is at ram[0x10000 + addr]
    uint8_t table_byte = ram[0x15C71 + index1];   // 0x7F5C71

    ram[0x3D] = table_byte;
    ram[0x3E] = 0;               // stz $3E

    // asl $3D / rol $3E — 16-bit shift left of (high=$3E, low=$3D)
    uint8_t low  = ram[0x3D];
    uint8_t high = ram[0x3E];    // 0
    bool carry = (low & 0x80) != 0;
    low  = (uint8_t)(low << 1);
    high = (uint8_t)((high << 1) | (carry ? 1 : 0));
    ram[0x3D] = low;
    ram[0x3E] = high;

    // ldx $3D (16-bit) → index2 = (high << 8) | low
    uint16_t index2 = ((uint16_t)high << 8) | low;

    // lda $0EDB,x (DB=$7E) → ram[0x0EDB + index2]
    uint8_t result = ram[0x0EDB + index2];
    result &= 0x83;              // and #$83
    return result;               // jmp @c00a → plx / rts
}

// PITFALLS:
//   1 (DB=$7E required for $0EDB,x access — we use absolute ram[] offsets)
//   6 (mode A 8-bit assumed; mf=true)
//   7 (asl/rol on memory in 8-bit mode — carry propagation handled with
//      uint8_t casts and explicit carry flag)
//   8 (mode heritage: mf=true, xf=false inherited from field module callers)
//
// HELPERS: none (no sub-routine calls)
//
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0C=1, 0x0E=1
//   output_ram:  none   (result returned in A; $3D/$3E are internal temporaries)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//
// REVERSED_FUNCTION: field::_00bfe3 ($BF:E3)