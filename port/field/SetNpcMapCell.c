#include "snes/snes.h"
void SetNpcMapCell_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    const uint16_t dp = snes->cpu->dp;
    const uint8_t y = ram[(uint16_t)(dp + 0x0E)];
    const uint8_t hi = (uint8_t)(y >> 3);
    const uint8_t lo = (uint8_t)((uint8_t)((y & 7) << 5) + ram[(uint16_t)(dp + 0x0C)]);
    ram[(uint16_t)(dp + 0x3E)] = hi;
    ram[(uint16_t)(dp + 0x3D)] = lo;
    ram[0x14C00 + (uint16_t)((hi << 8) | lo)] =
        (uint8_t)(ram[(uint16_t)(dp + 0xAE)] | 0x80);
}
// SPIKE_COMPARE: region
// CONTRACT:
//   inputs_ram:  0x060C=1, 0x060E=1, 0x06AE=1
//   output_ram:  0x063D=1, 0x063E=1
//   entry_mode:  mf=true, xf=false, dp=0x0600, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::SetNpcMapCell ($00:C347)
