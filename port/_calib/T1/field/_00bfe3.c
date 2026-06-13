#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$BF, DP=0
// This routine looks up a bitmask based on coordinate values in DP.
// It uses a table at $7F:5C71 to map a value to an index, then 
// accesses another table (relative to PC) to retrieve the final mask.
//
// Logic:
// 1. Read DP:$0C and DP:$0E. If either is negative (bit 7 set), return 0.
// 2. Use DP:$0C as index into table at $7F:5C71 to get a new value.
// 3. Shift that value left by 1 (16-bit result).
// 4. Use that result as index into table at $0EDB (relative to PC).
// 5. Return (A = result & 0x83).
static uint16_t field_00bfe3_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // Note: Registers X/Y are preserved (phx/plx)
    uint8_t val_0c = ram[cpu->dp + 0x0C];
    if (val_0c & 0x80) { // bmi @c008
        return 0;
    }

    uint8_t val_0e = ram[cpu->dp + 0x0E];
    if (val_0e & 0x80) { // bmi @c008
        return 0;
    }

    // The asm uses DP:$3D and $3E as scratch for a 16-bit value
    // ldx $3d / lda $7f5c71,x / sta $3d / stz $3e
    uint8_t index = val_0c;
    uint16_t lookup_val = ram[0x7F5C71 + index]; // Absolute address $7F:5C71
    
    // asl $3d / rol $3e (16-bit shift left)
    uint16_t shifted_idx = (uint16_t)lookup_val << 1;

    // ldx $3d / lda $0edb,x
    // The address $0edb is PC-relative or absolute in the bank.
    // In the context of the disassembly, $0EDB refers to the data area in bank $BF.
    // We use the bank-relative offset for the table lookup.
    uint8_t table_val = ram[0xBF000 + 0x0EDB + shifted_idx]; 
    
    return (uint16_t)(table_val & 0x83);
}

// PITFALLS: 6 (A 8-bit vs 16-bit: routine uses 8-bit A but performs 16-bit 
// shift via two 8-bit registers $3D/$3E), 7 (Arithmetic truncation: 
// the shift is handled as uint16_t to prevent 8-bit overflow loss).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x000C=1, 0x000E=1, 0x7F5C71=1 (Bank $7F)
//   output_ram:  none (returns value in A)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBF
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (result is in A, not RAM)
// REVERSED_FUNCTION: field::_00bfe3 ($BF:E3)