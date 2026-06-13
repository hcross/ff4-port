#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Input:  ram[$0C] and ram[$0E] treated as signed 8-bit values
// Output: ram[$3D] = result_lo, ram[$3E] = result_hi
//         (16-bit value stored little-endian at $3D/$3E)
//
// Logic:
//   if ($0c < 0 || $0e < 0) {
//       result = 0
//   } else {
//       x = $0c
//       byte_val = $7f5c71,x
//       $3d = byte_val
//       $3e = 0
//       $3d <<= 1      ; 16-bit shift left (with carry to $3e)
//       x = $3d        ; (16-bit X)
//       result = $0edb,x & 0x83
//   }
static void _00bfe3_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    int8_t a = (int8_t)ram[0x0C];  // lda $0c / bmi check
    if (a < 0) goto neg;           // bmi @c008
    ram[0x3D] = a;                 // sta $3d
    int8_t e = (int8_t)ram[0x0E];  // lda $0e / bmi check
    if (e < 0) goto neg;           // bmi @c008
    ram[0x3E] = e;                 // sta $3e
    uint16_t x = ram[0x3D];        // ldx $3d
    uint8_t byte_val = ram[0x7f5c71 + x]; // lda $7f5c71,x
    ram[0x3D] = byte_val;          // sta $3d
    ram[0x3E] = 0;                 // stz $3e
    uint16_t val = ((uint16_t)ram[0x3E] << 8) | ram[0x3D]; // 16-bit value
    val <<= 1;                     // asl $3d / rol $3e combined
    ram[0x3D] = val & 0xFF;
    ram[0x3E] = (val >> 8) & 0xFF;
    x = val & 0xFFFF;              // ldx $3d (16-bit X)
    uint8_t result = ram[0x0EDB + x]; // lda $0edb,x
    result &= 0x83;                // and #$83
    ram[0x3D] = result;            // sta $3d (final result in A)
    ram[0x3E] = 0;                 // upper byte zeroed
    return;
neg:;                              // @c008:
    ram[0x3D] = 0;                 // lda #$00 / sta $3d
    ram[0x3E] = 0;                 // upper byte zeroed
}

// PITFALLS: 1 (DB=$7E assumed), 6 (A 8-bit, X 16-bit), 7 (16-bit shift
// must truncate properly), 9 (upper byte B preserved in 8-bit A loads)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x0C=1, 0x0E=1
//   output_ram:  0x3D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
// REVERSED_FUNCTION: field::_00bfe3 ($BF:E3)