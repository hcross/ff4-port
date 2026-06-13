#include "snes/snes.h"

static uint8_t RotateWord_c(Snes *snes, uint8_t a) {
    uint8_t *ram = snes->ram;
    uint8_t c = snes->cpu->c ? 1 : 0;
    
    ram[0x09] = a;
    ram[0x07] = a;
    
    // ror $09
    uint8_t new_c = ram[0x09] & 1;
    ram[0x09] = (c << 7) | (ram[0x09] >> 1);
    c = new_c;
    
    // ror $08
    new_c = ram[0x08] & 1;
    ram[0x08] = (c << 7) | (ram[0x08] >> 1);
    c = new_c;
    
    // ror $07
    new_c = ram[0x07] & 1;
    ram[0x07] = (c << 7) | (ram[0x07] >> 1);
    c = new_c;
    
    uint8_t result = ram[0x08];
    snes->cpu->a = result;
    snes->cpu->c = c;
    snes->cpu->z = (result == 0);
    snes->cpu->n = (result & 0x80) != 0;
    return result;
}