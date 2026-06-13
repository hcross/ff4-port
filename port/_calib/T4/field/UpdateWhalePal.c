#include "snes/snes.h"

static void UpdateWhalePal_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    if (ram[0x1704] != 0x06) {   // lda $1704 / cmp #$06 / bne @c3d9
        return;
    }
    uint8_t a = ram[0x007A];
    a >>= 1;                     // lsr
    a >>= 1;                     // lsr
    a &= 0x0E;                   // and #$0e
    uint16_t x = a;              // tax (X 16-bit, high byte cleared)
    // Load from WhalePal table in ROM. But wait - how do we access ROM?