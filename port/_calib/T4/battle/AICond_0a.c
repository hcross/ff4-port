#include "snes/snes.h"

static void AICond_0a_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t idx = (uint8_t)((ram[0xD2] - 5) << 1); // sec, lda $d2, sbc #$05, asl
    // Wait, sbc in 8-bit: A = ram[0xD2] - 5 - (1 - C). Since sec sets C=1, borrow is 0.
    // So A = ram[0xD2] - 5.
    // Then asl shifts left. In 8-bit mode, (uint8_t)((ram[0xD2] - 5) << 1).
    uint16_t word = read16(ram, 0x34D4 + idx); // lda $34d4,x / ora $34d5,x
    if (word == 0) return; // beq @bf04
    uint8_t hi = ram[0x34D5 + idx]; // lda $34d5,x
    if ((hi & 0xC0) != 0) return; // and #$c0 / bne @bf04
    ram[0xDE]++; // inc $de
}