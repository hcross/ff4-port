#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (Direct Page mode), DP=0
// This routine calculates the battle background ID and flags based on
// cave/water status and game state, then writes the result to HW register $1802.
static void GetBattleBG_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Use cave w/ water battle bg
    uint8_t temp06 = (uint8_t)((ram[0xA2] & 0x20) << 1); // lda $a2 / and #$20 / asl / sta $06
    
    // Use alternate battle bg palette
    uint8_t temp_alt = (uint8_t)((ram[0x0FDB] & 0x40) >> 1); // lda $0fdb / and #$40 / lsr
    temp06 |= temp_alt; // ora $06 / sta $06

    // Final battle bg id and magnetization flag
    uint8_t result = (ram[0x0FDB] & 0x8F) | temp06; // lda $0fdb / and #$8f / ora $06
    
    // Write to HW register $1802 (Direct Page = 0)
    ram[0x1802] = result; 
}

// PITFALLS: 7 (8-bit shift truncation: explicitly cast to uint8_t to ensure
// 8-bit semantics match the 65816 A-register size).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00A2=1, 0x0FDB=1
//   output_ram:  0x1802=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetBattleBG ($8C:D4)