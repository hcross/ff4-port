#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$AB, DP=0
// This routine calculates a VRAM pointer for BG1 based on screen/layer offsets.
// It uses DP registers $0C and $0E as inputs and writes intermediate 
// results to DP $18-$19.
static void GetBG1VRAMPtr_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Memory offsets relative to DP (0x00) in DB $AB. 
    // Note: The ASM uses absolute DP addressing. Since DB is $AB, 
    // $0E refers to snes->ram[0xAB0E].
    
    uint8_t val0e = ram[0xAB0E];
    uint8_t val0c = ram[0xAB0C];

    // lda $0e / and #$0f / sta $19 / stz $18
    uint8_t reg19 = val0e & 0x0F;
    uint8_t reg18 = 0;

    // lsr $19 / ror $18 (x2) -> Right shift 16-bit value (reg18:reg19) by 2
    // Since reg18 is 0, this is just reg19 >> 2
    reg19 >>= 2; 
    // reg18 remains 0 because no bits shifted from reg19 into reg18 
    // (the high bit of reg19 is 0 due to 'and #$0f')

    // lda $0c / and #$0f / asl
    uint8_t temp = (val0c & 0x0F) << 1;

    // clc / adc $18 / sta $18
    reg18 = (uint8_t)(temp + reg18); // Pitfall 7: wrap to 8-bit

    // lda $19 / clc / adc #$18 / sta $19
    reg19 = (uint8_t)(reg19 + 0x18); // Pitfall 7: wrap to 8-bit

    // lda $0c / and #$10 / beq @ab5d
    if ((val0c & 0x10) != 0) {
        // lda $19 / clc / adc #$04 / sta $19
        reg19 = (uint8_t)(reg19 + 0x04); // Pitfall 7: wrap to 8-bit
    }

    // Store results back to DP for consistency with ASM side-effects
    ram[0xAB18] = reg18;
    ram[0xAB19] = reg19;

    // ldx $18 / rts
    snes->cpu->x = reg18;
}

// PITFALLS: 7 (Arithmetic truncation: used (uint8_t) casts for ADC)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xAB0C=1, 0xAB0E=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xAB
//   entry_flags: z=auto, n=auto
//   custom_spike: yes (outputs to register X)

// REVERSED_FUNCTION: field::GetBG1VRAMPtr ($AB:2F)