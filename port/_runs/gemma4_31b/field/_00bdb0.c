#include "snes/snes.h"

// This routine calculates a coordinate offset or range check based on 
// data indexed by the value at $af. It processes two 16-bit values from 
// a table at $0904, transforms them via shifts and arithmetic, 
// and stores the result in $18 and $1a. It also sets a flag at $d7.
static void _00bdb0_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Save registers (phx, phy) - implicit in C as we don't mutate snes->cpu->x/y
    ram[0xD7] = 0; 
    uint16_t x = read16(ram, 0xAF); // ldx $af (X is 16-bit by field convention)

    // Process first value
    // longa: A becomes 16-bit. 
    // lda $0904,x: reads word at ($0904 + x)
    uint16_t val1 = read16(ram, 0x0904 + x);
    val1 &= 0x00FF;                      // and #$00ff
    val1 = (uint16_t)(val1 << 4);       // asl4 (Shift left 4)
    
    // clc / adc $0c / sec / sbc $5a
    // Note: $0c and $5a are DP-relative (DP=0)
    uint16_t term1 = (uint16_t)(val1 + ram[0x0C]); 
    uint16_t result1 = (uint16_t)(term1 - ram[0x5A]);
    result1 &= 0x03FF;                  // and #$03ff
    ram[0x18] = result1 & 0xFF;         // sta $18 (low byte)
    ram[0x19] = (result1 >> 8) & 0xFF;  // sta $18 (high byte - implied by longa)

    if (result1 >= 0x0100) {            // cmp #$0100 / bcs @bdec
        ram[0xD7]++;                     // inc $d7
        goto label_bdee;
    }

    // Process second value
    uint16_t val2 = read16(ram, 0x0906 + x);
    val2 &= 0x00FF;                      // and #$00ff
    val2 = (uint16_t)(val2 << 4);       // asl4
    
    uint16_t term2 = (uint16_t)(val2 + ram[0x0E]); 
    uint16_t result2 = (uint16_t)(term2 - ram[0x5C]);
    result2 &= 0x03FF;                  // and #$03ff
    ram[0x1A] = result2 & 0xFF;         // sta $1a
    ram[0x1B] = (result2 >> 8) & 0xFF;  // sta $1a (high byte)

    if (result2 < 0x00F0) {             // cmp #$00f0 / bcc @bdee
        goto label_bdee;
    }

    ram[0xD7]++;                        // inc $d7

label_bdee:;
    // lda #0 / shorta (A is now 8-bit 0)
    // ply / plx / rts (stack restored)
}

// PITFALLS: 6 (Mode A switches from 8-bit to 16-bit via longa, 
// then back to 8-bit via shorta), 7 (Arithmetic truncation: using uint16_t 
// to simulate 16-bit 65816 registers).
// HELPERS: read16()
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xAF=2, 0x0C=1, 0x5A=1, 0x0E=1, 0x5C=1, 0x0904=2, 0x0906=2
//   output_ram:  0x18=2, 0x1A=2, 0xD7=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBD
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00bdb0 ($BD:B0)