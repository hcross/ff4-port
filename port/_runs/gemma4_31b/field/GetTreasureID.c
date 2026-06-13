#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9B, DP=0
// This routine calculates a treasure index based on a seed/location value
// and an offset flag.
//   in : ram[0x08FC] (seed/loc), ram[0x1701] (offset flag)
//   out: cpu->a = result (8-bit), cpu->y = index (8-bit)
static void GetTreasureID_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // lda $08fc / lsr3 / tax
    // Note: lsr3 is a macro for 3 shifts. In 8-bit mode, this is (val >> 3).
    uint8_t seed = ram[0x08FC];
    uint16_t x = (uint16_t)(seed >> 3); 
    
    // lda $1701 / beq @9b53
    uint8_t flag = ram[0x1701];
    if (flag != 0) {
        // txa / clc / adc #$20 / tax
        x = x + 0x20;
    }
    
    // lda $08fc / and #$07 / inc / tay
    uint8_t index = (uint8_t)(ram[0x08FC] & 0x07);
    index++;
    
    snes->cpu->y = index;
    snes->cpu->a = index; // The routine ends with 'tay / rts'. 
                          // In 65816, TAY doesn't change A. 
                          // A remains the result of the 'inc' operation.
}

// PITFALLS: 7 (Arithmetic truncation: used uint8_t for the index calc to 
// ensure 8-bit wrap-around parity with the 65816 'inc' instruction).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x08FC=1, 0x1701=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9B
//   entry_flags: z=auto, n=auto
//   custom_spike: yes (outputs are in registers A and Y)

// REVERSED_FUNCTION: field::GetTreasureID ($9B:42)