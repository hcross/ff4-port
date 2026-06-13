#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9B, DP=0
// Logic:
//   1. Call GetTreasureID to find the treasure index (returned in X).
//   2. Load a byte from a table at $12A0 + X into a temporary storage ($07).
//   3. Right-shift that byte Y times.
//   4. Result is always 0 because it performs "lda #0 / adc #0" (effectively clr A) 
//      immediately after the loop regardless of the shift result.
static uint16_t CheckTreasureSwitch_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // jsr GetTreasureID: returns treasure ID in X
    get_treasure_id_emu(snes);
    uint16_t x = snes->cpu->x;

    // lda $12a0,x / sta $07
    // Note: Using DP=0, $12a0 is an absolute address in the current DB ($9B)
    // but since the harness uses a flat ram array for WRAM and the prompt 
    // implies the logic is within the mapping, we access the provided offset.
    uint8_t val = ram[0x12A0 + x];
    ram[0x07] = val;

    // Loop @9b15: lsr $07 / dey / bne @9b15
    uint8_t y = snes->cpu->y;
    for (uint8_t i = 0; i < y; i++) {
        ram[0x07] = (uint8_t)(ram[0x07] >> 1); // Pitfall 7: truncate to 8-bit
    }

    // lda #$00 / adc #$00 / rts
    // The result of the loop is ignored; the function always returns 0 in A.
    return 0;
}

// PITFALLS: 7 (lsr $07 truncated to 8-bit)
// HELPERS: get_treasure_id_emu(snes) — delegates GetTreasureID @ $9B:42
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=8bits
//   inputs_ram:  0x12A0=<1> (indexed by X from GetTreasureID)
//   output_ram:  0x07=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9B
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::CheckTreasureSwitch ($9B:0D)