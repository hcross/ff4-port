#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x7E, DP=0
// Initializes star sprites for the solar system cutscene.
// Uses RNGTbl to scatter 56 stars on screen, setting size and calling DrawSolarSystemSprite.
static void InitStars_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x50] = 0x48; // lda #$48 / sta $50
    // clr_ax → tdc / tax (A = X = DP = 0)
    ram[0x7D27] = 0;
    uint16_t x = 0;
    uint8_t a = 0;
loop_e24d:
    ram[0x7D27]++;         // inc $7d27
    uint8_t rng1 = ram[0x89A0 + x]; // lda f:RNGTbl,x (0x89A0 = base of RNGTbl in ROM mirror)
    uint8_t y = rng1;
    ram[0x4B] = y;         // sty $4b
    uint8_t rng2 = ram[0x89A0 + y]; // lda f:RNGTbl,x (X = Y now)
    ram[0x4D] = rng2;      // sty $4d
    uint8_t temp = rng2;
    uint8_t counter = ram[0x7D27];
    uint8_t and_result = counter & 0x07;
    uint8_t a_val;
    if (and_result == 0) { // beq @e26b
        a_val = 1;         // lda #1
    } else {
        a_val = 0;
    }
    a_val = (uint8_t)(a_val + 0x50); // clc / adc #$50
    ram[0x4F] = a_val;     // sta $4f
    ram[0x51] = 1;         // lda #$01 / sta $51
    draw_solar_system_sprite_emu(snes); // jsr DrawSolarSystemSprite
    x += 2;                // inx2
    a++;                   // inc (A is 8-bit)
    if (a != 0x38) {       // cmp #$38 / bne loop
        goto loop_e24d;
    }
    // rts
}

// PITFALLS: 1 (DB=$7E assumed), 6 (A is 8-bit, X is 16-bit), 7 (8-bit ADC truncation),
//           9 (clr_ax is tdc/tax, not lda #0/ldx #0 — but DP=0 here so it's safe)
// HELPERS: draw_solar_system_sprite_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::InitStars ($E2:0047)