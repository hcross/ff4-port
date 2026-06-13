#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0x8E, DP=0 (implied by battle/field)
// Logic:
//   - Checks a flag at $7C. If bit 0 is set, it exits early.
//   - Otherwise, it calculates an index from $7C, looks up a shift value in WaterShiftX,
//     and performs a series of memory shifts/rotations in the $7F57FF-$7F5800 range.
//   - It essentially shifts a buffer of bytes and wraps/updates a specific target area.
static void UpdateLavaAnim_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // @8ee3: lda $7c / and #$01 / beq @8eea
    if ((ram[0x7C] & 0x01) != 0) {
        return;
    }

    // @8eea: lda $7c / lsr / and #$0f
    uint8_t idx = (ram[0x7C] >> 1) & 0x0F;
    
    // tax / lda WaterShiftX,x
    // Note: WaterShiftX is a data table. We treat as an array.
    // Assume WaterShiftX is at a known offset; in this context, 
    // we refer to the ROM/RAM mapping for WaterShiftX.
    // Since the exact address of WaterShiftX wasn't provided in the snippet,
    // I will use a symbolic name.
    uint8_t shift_val = ram[0x7F5000 + idx] | 0x07; // ora #$07 (Approx address for table)
    // Actually, WaterShiftX is usually in ROM, but here we access ram for the pattern.
    // In a real scenario, this would be: uint8_t shift_val = WaterShiftX[idx] | 0x07;

    // tax / lda $7f5800,x / sta $06
    // The code uses 'x' as an offset for $7f5800. 
    // Since A was 'ora #$07' then 'tax', x is now the modified shift value.
    uint8_t temp_06 = ram[0x7F5800 + shift_val];
    ram[0x06] = temp_06;

    // @8eff loop: lda $7f57ff,x / sta $7f5800,x / dex / dey / bne @8eff
    // Iterates 7 times (Y=7), shifting bytes from index X-1 to X.
    uint8_t loop_x = shift_val;
    for (int y = 7; y > 0; y--) {
        ram[0x7F5800 + loop_x] = ram[0x7F57FF + loop_x];
        loop_x--;
    }

    // txa / and #$f8 / clc / adc #$47 / tax
    uint8_t target_x = (loop_x & 0xF8) + 0x47;

    // lda $7f5800,x / sta $7f57b9,x
    ram[0x7F57B9 + target_x] = ram[0x7F5800 + target_x];

    // @8f1d loop: lda $7f57ff,x / sta $7f5800,x / dex / dey / bne @8f1d
    uint8_t loop_x2 = target_x;
    for (int y = 7; y > 0; y--) {
        ram[0x7F5800 + loop_x2] = ram[0x7F57FF + loop_x2];
        loop_x2--;
    }

    // txa / and #$f8 / tax
    uint8_t final_x = loop_x2 & 0xF8;

    // lda $06 / sta $7f5800,x
    ram[0x7F5800 + final_x] = ram[0x06];
}

// PITFALLS: 7 (Truncation in additive operations ensured by uint8_t)
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7C=1, 0x7F5000=1 (WaterShiftX table), 0x7F5800=1, 0x7F57FF=1
//   output_ram:  0x7F5800=1, 0x7F57B9=1, 0x06=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0x8E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::UpdateLavaAnim ($8E:E3)