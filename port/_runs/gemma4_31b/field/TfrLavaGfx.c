#include "snes/snes.h"

// Logic:
// Transfers lava graphics (Water tiles) in two passes.
// First pass: Sets VRAM address based on WaterShiftX table index from ram[0x7C].
// Second pass: Sets VRAM address with a 0x40 offset to the same table value.
// Increments the animation frame counter at $7C.
static void TfrLavaGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // First Pass
    // lda #$80 / sta $2115
    snes->ram[0x2115] = 0x80; 
    
    // lda $7c / lsr / and #$0f / tax
    uint8_t shift_idx = (uint8_t)((ram[0x7C] >> 1) & 0x0F);
    
    // lda WaterShiftX,x / tax / sta $2116
    // WaterShiftX is located at 0x0000 in the current data bank (0x8F)
    // Since we access ram directly, we use the bank-relative offset
    uint8_t shift_val = ram[0x8F00 + shift_idx]; // Note: WaterShiftX is at $8F:0000
    snes->ram[0x2116] = shift_val;
    
    // lda #$38 / sta $2117
    snes->ram[0x2117] = 0x38;
    
    // jsr TfrWaterTiles
    TfrWaterTiles_emu(snes);

    // Second Pass
    // lda $7c / lsr / and #$0f / tax
    shift_idx = (uint8_t)((ram[0x7C] >> 1) & 0x0F);
    
    // lda WaterShiftX,x / clc / adc #$40 / tax / sta $2116
    uint8_t offset_val = (uint8_t)(ram[0x8F00 + shift_idx] + 0x40); // Pitfall 7
    snes->ram[0x2116] = offset_val;
    
    // lda #$38 / sta $2117
    snes->ram[0x2117] = 0x38;
    
    // jsr TfrWaterTiles
    TfrWaterTiles_emu(snes);

    // inc $7c
    ram[0x7C]++;
}

// PITFALLS: 7 (8-bit truncation for addition)
// HELPERS: TfrWaterTiles_emu(snes) — delegates TfrWaterTiles @ 8E0E
//
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7C=1, 0x8F00=16
//   output_ram:  0x7C=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x8F
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::TfrLavaGfx ($8F:34)