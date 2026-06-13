#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0xCB, DP=0
// Logic:
//   1. Calculate offset into MapAnimGfx based on BGAnimTbl index at 0x0FDD.
//   2. Copy 16 bytes of GFX data.
//   3. Enter a loop that copies blocks of 2 bytes (1 data, 1 zero) until 0x800 bytes are written.
//   4. Note: The loop logic involves a 16-byte window (and #$0f) and a total length check of 0x800.
static void LoadBGAnimGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Initial index from RAM
    uint8_t index = ram[0x0FDD];
    
    // Calculate start pointer: (BGAnimTbl[index] >> 1) + BGAnimTbl[index]
    // The ASM does: lsr $3e / ror $3d (shifts a 16-bit value formed by $3e:$3d)
    // then adc BGAnimTbl[index]. 
    // effectively: (val / 2) + val = 1.5 * val
    uint8_t tbl_val = snes->rom[0xCB0000 + 0x0000 /* f:BGAnimTbl offset unknown, using relative */]; 
    // Wait, f:BGAnimTbl and f:MapAnimGfx are ROM labels. 
    // For this translation, we treat these as reads from the ROM bank.
    
    // Correct interpretation of the arithmetic:
    // lda f:BGAnimTbl,x -> sta $3e / stz $3d -> lsr $3e / ror $3d
    // This is effectively dividing the 16-bit value BGAnimTbl[index] by 2.
    // Since $3d was 0, it's just (uint8_t)tbl_val >> 1.
    // Then: lda $3e (result of shift) / clc / adc tbl_val -> sta $3e
    // result = (tbl_val >> 1) + tbl_val.
    
    uint8_t val = snes->rom[0xCB0000 + 0x/*BGAnimTbl*/ + index]; // Simplified ROM access
    uint16_t offset = (uint16_t)((val >> 1) + val);
    
    uint16_t x = offset;
    uint16_t y = 0;

    // @cb23 Loop: Copy 16 bytes
    do {
        ram[0x5000 + y] = snes->rom[0xCB0000 + 0x/*MapAnimGfx*/ + x];
        x++;
        y++;
        if ((y & 0x0F) != 0) {
            // continue loop
        } else {
            break; // break to @cb31
        }
    } while (1);

    // @cb31 Loop: Copy in pairs (Data, 0)
    while (1) {
        ram[0x5000 + y] = snes->rom[0xCB0000 + 0x/*MapAnimGfx*/ + x];
        x++;
        y++;
        ram[0x5000 + y] = 0;
        y++;
        
        if ((y & 0x0F) != 0) {
            // continue loop
        } else {
            // Check if we reached 0x800 bytes
            if (y != 0x0800) {
                // jump back to @cb23 logic (the 16-byte copy)
                // In the ASM, this is: cpy #$0800 / bne @cb23
                // We simulate the jump by continuing the logic
                while ((y & 0x0F) != 0) {
                    ram[0x5000 + y] = snes->rom[0xCB0000 + 0x/*MapAnimGfx*/ + x];
                    x++;
                    y++;
                }
                // After the 16-byte block, we hit @cb31 again.
                // To match exactly, we'd loop.
                continue; 
            } else {
                break; // Done
            }
        }
    }
}

// PITFALLS: 7 (8-bit arithmetic truncation on adc), 8 (mf=true inherited)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0FDD=1
//   output_ram:  0x5000=2048
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xCB
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadBGAnimGfx ($CB:01)