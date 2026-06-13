#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9B, DP=0
// Logic:
// 1. Get Treasure ID (returned in A).
// 2. Calculate bitmask based on Y: find the lowest set bit of (Y + 1).
//    Essentially: 1 << (trailing zeros of (Y+1)).
// 3. Check if the bit is already set in the treasure switch map at $12A0,x.
// 4. If not set, set it.
// 5. Return the original bit status in A.
static uint8_t SetTreasureSwitch_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // jsr GetTreasureID
    uint8_t treasure_id = get_treasure_id_emu(snes);
    
    // Calculate bitmask based on Y (The loop @9b24)
    // a = 1, while (y != 0) { y--, a <<= 1 }
    uint8_t mask = 1;
    uint16_t y_val = snes->cpu->y;
    while (y_val != 0) {
        y_val--;
        mask = (uint8_t)(mask << 1); // Pitfall 7: truncate to 8-bit
    }
    
    // sta $06
    uint8_t temp_06 = mask; 
    
    // lda $12a0,x / and $06 / sta $07
    uint16_t addr = (uint16_t)(0x12A0 + snes->cpu->x);
    uint8_t current_val = ram[addr];
    uint8_t bit_status = current_val & temp_06;
    
    if (bit_status == 0) { // bne @9b3f (inverted)
        // lda $12a0,x / clc / adc $06 / sta $12a0,x
        ram[addr] = (uint8_t)(current_val + temp_06); // Pitfall 7: truncate
    }
    
    return bit_status; // lda $07 / rts
}

// PITFALLS: 7 (Shift/Add truncation to 8-bit to match 65816 Mode A 8-bit)
// HELPERS: get_treasure_id_emu(snes) — delegates GetTreasureID @ $9B:42
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=16
//   inputs_ram:  0x12A0=1 (indexed by X)
//   output_ram:  0x12A0=1 (indexed by X)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9B
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::SetTreasureSwitch ($9B:1F)