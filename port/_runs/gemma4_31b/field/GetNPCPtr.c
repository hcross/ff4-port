#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$BB, DP=0
// This routine converts an NPC index into a pointer offset, 
// handling a specific table alignment.
// 
// Logic:
// 1. Mask index to 7 bits (0-127).
// 2. The loop from @bb12 to @bb1e calculates: (index * 16) - (index * 15)
//    Actually, it is a subtraction loop that subtracts 1 from X and adds 15 to A,
//    effectively calculating (index * 15).
// 3. It then checks if the NPC is active/valid via indices at $0903,x and $0905,x.
// 4. If valid, it returns the value at $0902,x minus 1. Otherwise returns 0xFF.
static uint8_t GetNPCPtr_c(Snes *snes, uint8_t npc_index) {
    uint8_t *ram = snes->ram;
    
    // and #$7f / tax
    uint16_t x = (uint16_t)(npc_index & 0x7F);
    uint8_t a = 0; // lda #$00

    // Loop @bb12: calculates index * 15
    while (x != 0) { // cpx #$0000 / beq @bb1e
        x--;         // dex
        a = (uint8_t)(a + 0x0F); // clc / adc #$0f (Pitfall 7: truncate to 8-bit)
    }

    // tax (X now holds the result of index * 15)
    x = (uint16_t)a;
    
    // The code uses DP-relative addressing: $0903,x etc. 
    // Since DP=0, these are absolute addresses.
    uint8_t val03 = ram[0x0903 + x]; // lda $0903,x
    uint8_t val05 = ram[0x0905 + x]; // ora $0905,x
    
    if ((val03 | val05) == 0) {      // bne @bb2c
        return 0xFF;                // lda #$ff
    }

    uint8_t result = ram[0x0902 + x]; // lda $0902,x
    return (uint8_t)(result - 1);   // dec / rts
}

// PITFALLS: 7 (Arithmetic truncation in 8-bit mode: adc #$0f result wrapped to uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x0902=1, 0x0903=1, 0x0905=1 (relative to calculated X)
//   output_ram:  none (returns result in A)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBB
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: no
// REVERSED_FUNCTION: field::GetNPCPtr ($BB:0D)