#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C3 (ROM/Data), DP=0
// Purpose: Updates the Babil palace palette based on map location ($1700) 
// and flashing state ($1288).
// Logic:
//   - If map != 0 and map != 1, exit.
//   - If map == 0, target BG palette 0 (index $001e).
//   - If map == 1, target BG palette 3 (index $007e).
//   - If flashing bit ($1288 & 4) is set:
//       pal_low = asl($7a), pal_high = ~pal_low
//   - Else:
//       pal_low = 0, pal_high = 0
static void UpdateBabilPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t map = ram[0x1700];

    if (map >= 2) {                // cmp #$02 / bcs @c419
        return;
    }

    uint16_t pal_idx;
    if (map == 0) {                // cmp #$00 / bne @c3fb
        pal_idx = 0x001E;         // ldx #$001e
    } else {                      // @c3fb
        pal_idx = 0x007E;         // ldx #$007e
    }

    if ((ram[0x1288] & 0x04) == 0) { // and #$04 / beq @c413
        // Pitfall 7: 8-bit truncation for STZ on memory offsets
        ram[0x0CDB + pal_idx] = 0;
        ram[0x0CDC + pal_idx] = 0;
    } else {
        uint8_t val = ram[0x7A];
        uint8_t shifted = (uint8_t)(val << 1); // asl (Pitfall 7)
        ram[0x0CDB + pal_idx] = shifted;
        ram[0x0CDC + pal_idx] = (uint8_t)(~shifted); // eor #$ff
    }
}

// PITFALLS: 7 (8-bit truncation for 'asl' and 'sta' operations)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1700=1, 0x1288=1, 0x7A=1
//   output_ram:  0x0CDB=1 (indexed by pal_idx)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC3
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::UpdateBabilPal ($C3:EA)