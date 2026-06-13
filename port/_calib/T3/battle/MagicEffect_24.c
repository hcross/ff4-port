#include "snes/snes.h"

static void MagicEffect_24_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // 16-bit subtraction: $a4-$a5 = $2709-$2708 - $2707-$2706
    // Wait, let me re-read: lda $2709 / sbc $2707
    // In 16-bit mode, lda $2709 reads from $2709 and $270A
    // sbc $2707 reads from $2707 and $2708
    
    uint16_t val1 = read16(ram, 0x2709);
    uint16_t val2 = read16(ram, 0x2707);
    uint16_t result = val1 - val2;  // SEC + SBC = subtraction
    write16(ram, 0xA4, result);
    
    // Set bit 7 of high byte ($a5)
    ram[0xA5] |= 0x80;
    
    // Tail call to SetMagicStatus
    set_magic_status_emu(snes);
}