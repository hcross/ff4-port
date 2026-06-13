#include "snes/snes.h"

static void MagicEffect_24_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // 16-bit subtraction
    uint16_t val1 = read16(ram, 0x2709);
    uint16_t val2 = read16(ram, 0x2707);
    uint16_t result = val1 - val2; // sec means borrow clear, so normal subtraction
    
    write16(ram, 0xA4, result);
    
    // Now set bit 7 of high byte
    ram[0xA5] |= 0x80;
    
    // Tail call to SetMagicStatus
    set_magic_status_emu(snes);
}