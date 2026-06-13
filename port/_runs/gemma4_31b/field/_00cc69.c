#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$00 (Field module), DP=0
// This routine updates coordinates (likely position/velocity) for multiple entities.
// It iterates 16 times, incrementing values in two different RAM regions ($03B0 and $0410)
// based on a toggle flag at $7A and a limit of 8 entries for the second region.
static void _00cc69_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    for (uint16_t y = 0; y != 0x10; y += 4) { // iny4 / cpy #$10 / bne @cc6c
        // First block: always update $03B0 range
        // lda $03b0,y / adc #$28 / sta $03b0,y
        ram[0x03B0 + y] = (uint8_t)(ram[0x03B0 + y] + 0x28); // Pitfall 7: truncate to 8-bit
        // lda $03b1,y / adc #$29 / sta $03b1,y
        ram[0x03B1 + y] = (uint8_t)(ram[0x03B1 + y] + 0x29); // Pitfall 7: truncate to 8-bit

        // Check toggle flag at $7A
        if ((ram[0x7A] & 0x01) != 0) { // lda $7a / and #$01 / bne @cc9b
            continue;
        }

        // Check if y is within the first 8 entries (0, 4)
        // cpy #8 / bcs @cc9b
        if (y >= 8) { // Pitfall 3: bcs branches when Y >= 8
            continue;
        }

        // Second block: update $0410 range
        // lda $0410,y / adc #$28 / sta $0410,y
        ram[0x0410 + y] = (uint8_t)(ram[0x0410 + y] + 0x28); // Pitfall 7: truncate to 8-bit
        // lda $0411,y / adc #$2b / sta $0411,y
        ram[0x0411 + y] = (uint8_t)(ram[0x0411 + y] + 0x2B); // Pitfall 7: truncate to 8-bit
    }
}

// PITFALLS: 3 (CPY #8 / BCS: branches when Y >= 8), 7 (8-bit ADC truncation)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x03B0=1, 0x03B1=1, 0x0410=1, 0x0411=1, 0x007A=1
//   output_ram:  none (updates multiple indices in 0x03B0/0x0410)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: field::_00cc69 ($CC:69)