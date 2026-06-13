#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$BF, DP=0
// Purpose: Maps coordinates in DP to a bitmask using two lookup tables.
// Logic:
//   1. Validate coordinates in $0C and $0E are non-negative.
//   2. Look up index in table at $7F:5C71 using coord $0C.
//   3. Shift the result left by 1 (16-bit operation via $3D and $3E).
//   4. Use that result to look up a value in table at $00:0EDB.
//   5. Return (value & 0x83) in the accumulator.
static void _00bfe3_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // lda $0c / bmi @c008
    uint8_t coord_x = ram[0x0C];
    if ((int8_t)coord_x < 0) {
        cpu->a = 0; // @c008: lda #$00
        return;
    }

    // sta $3d / lda $0e / bmi @c008
    ram[0x3D] = coord_x;
    uint8_t coord_y = ram[0x0E];
    if ((int8_t)coord_y < 0) {
        cpu->a = 0; // @c008: lda #$00
        return;
    }

    // sta $3e
    ram[0x3E] = coord_y;

    // ldx $3d / lda $7f5c71,x / sta $3d
    // Note: X is 16-bit (xf=0). ldx $3d loads 16-bit from DP.
    uint16_t x_idx = read16(ram, 0x3D); 
    // Absolute address $7F:5C71 + x
    uint8_t table1_val = ram[0x5C71 + x_idx];
    ram[0x3D] = table1_val;

    // stz $3e / asl $3d / rol $3e
    ram[0x3E] = 0;
    uint16_t val16 = (uint16_t)ram[0x3D];
    val16 <<= 1; // Pitfall 7: Shift a 16-bit composite
    write16(ram, 0x3D, val16);

    // ldx $3d / lda $0edb,x
    uint16_t index2 = read16(ram, 0x3D);
    // Table at $00:0EDB (implied DB=0 or absolute)
    // Since the routine is in DB=$BF, this refers to $00:0EDB
    uint8_t table2_val = ram[0x0EDB + index2];

    // and #$83 / jmp @c00a
    cpu->a = (uint16_t)(table2_val & 0x83);
}

// PITFALLS: 6 (A 8-bit/X 16-bit heritage), 7 (Composite 16-bit shift via RAM $3D/$3E),
//            1 (DB handling for $7F:5C71 and $00:0EDB)
// HELPERS: read16, write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0C=1, 0x0E=1, 0x5C71=1, 0x0EDB=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBF
//   entry_flags: z=auto, n=auto
//   return_reg:  a=8
// REVERSED_FUNCTION: field::_00bfe3 ($BF:E3)