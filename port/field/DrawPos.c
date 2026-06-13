#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0, DP=0
// This routine writes debug coordinates (X, Y, MapID) to SNES hardware registers.
// It uses a pattern of splitting a value into high/low nibbles and converting 
// them to ASCII/mapped characters for a display device (likely a debug monitor).
//
// Note: @PosX, @PosY, @MapID constants are defined based on build flags.
// In the target implementation (everything8215/ff4), we use the debug offsets.
static void DrawPos_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Define constants based on provided ASM (.if LANG_EN .or DEBUG)
    const int ADDR_POS_X = 0x1706;
    const int ADDR_POS_Y = 0x1707;
    const int ADDR_MAP_ID = 0x1702;

    // A = $80 -> $2115
    ram[0x2115] = 0x80;
    // X = $2882 -> $2116 (16-bit)
    write16(ram, 0x2116, 0x2882);

    // Process X coordinate
    uint8_t valX = ram[ADDR_POS_X];
    
    // High nibble (lsr4)
    uint8_t xHigh = valX >> 4;
    if (xHigh < 10) { // cmp #10 / bcc @c291
        xHigh |= 0x80; // ora #$80
    } else {
        xHigh += 0x38; // adc #$38 (C is clear)
    }
    ram[0x2118] = xHigh;
    ram[0x2119] = 0x20;

    // Low nibble (and #$0f)
    uint8_t xLow = valX & 0x0F;
    if (xLow < 10) { // cmp #10 / bcc @c2a9
        xLow |= 0x80;
    } else {
        xLow += 0x38;
    }
    ram[0x2118] = xLow;
    ram[0x2119] = 0x20;

    // Clear registers (stz $2118, stz $2119)
    ram[0x2118] = 0;
    ram[0x2119] = 0;

    // Process Y coordinate
    uint8_t valY = ram[ADDR_POS_Y];

    // High nibble (lsr4)
    uint8_t yHigh = valY >> 4;
    if (yHigh < 10) {
        yHigh |= 0x80;
    } else {
        yHigh += 0x38;
    }
    ram[0x2118] = yHigh;
    ram[0x2119] = 0x20;

    // Low nibble (and #$0f)
    uint8_t yLow = valY & 0x0F;
    if (yLow < 10) {
        yLow |= 0x80;
    } else {
        yLow += 0x38;
    }
    ram[0x2118] = yLow;
    ram[0x2119] = 0x20;

    // Switch target address (ldx #$28c2 / stx $2116)
    write16(ram, 0x2116, 0x28C2);

    // Process Map ID
    uint8_t valMap = ram[ADDR_MAP_ID];

    // High nibble (lsr4)
    uint8_t mHigh = valMap >> 4;
    if (mHigh < 10) {
        mHigh |= 0x80;
    } else {
        mHigh += 0x38;
    }
    ram[0x2118] = mHigh;
    ram[0x2119] = 0x20;

    // Low nibble (and #$0f)
    uint8_t mLow = valMap & 0x0F;
    if (mLow < 10) {
        mLow |= 0x80;
    } else {
        mLow += 0x38;
    }
    ram[0x2118] = mLow;
    ram[0x2119] = 0x20;
}

// PITFALLS: 3 (CMP/BCC logic: branch if A < 10), 7 (8-bit truncation: 
// implicit in uint8_t usage for arithmetic)
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1706=1, 0x1707=1, 0x1702=1
//   output_ram:  none (writes to HW registers $2115-$2119)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (outputs to HW regs, not standard WRAM)

// REVERSED_FUNCTION: field::DrawPos ($C2:76)