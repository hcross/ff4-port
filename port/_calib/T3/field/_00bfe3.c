#include "snes/snes.h"

static void _00bfe3_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    uint8_t v0c = ram[0x0C];
    if (v0c & 0x80) {           // bmi @c008
        snes->cpu->a = 0;       // lda #$00 (only low byte matters for return)
        return;                 // actually we need plx first...
    }
    
    ram[0x3D] = v0c;            // sta $3d
    
    uint8_t v0e = ram[0x0E];
    if (v0e & 0x80) {           // bmi @c008
        snes->cpu->a = 0;
        return;
    }
    
    ram[0x3E] = v0e;            // sta $3e
    
    uint16_t x = read16(ram, 0x3D);  // ldx $3d (16-bit)
    uint8_t table_val = ram[0x10000 + ((0x5C71 + x) & 0xFFFF)];  // lda $7f5c71,x
    ram[0x3D] = table_val;      // sta $3d
    ram[0x3E] = 0;              // stz $3e
    
    // asl $3d / rol $3e — 16-bit left shift
    uint16_t val = read16(ram, 0x3D);
    val <<= 1;
    write16(ram, 0x3D, val);
    
    x = read16(ram, 0x3D);     // ldx $3d
    uint8_t result = ram[(0x0EDB + x) & 0xFFFF];  // lda $0edb,x
    result &= 0x83;            // and #$83
    
    snes->cpu->a = result;      // return value in A
}