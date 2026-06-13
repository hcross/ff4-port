#include "snes/snes.h"

static uint8_t _00bfe3_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *c = snes->cpu;
    
    uint16_t saved_x = c->x; // phx
    // Or just use a local variable and restore at end.
    // Since plx restores x, we can save it locally.
    
    uint8_t a = ram[0x000C];
    if ((int8_t)a < 0) { // bmi
        a = 0;
        c->x = saved_x; // plx
        return a;
    }
    ram[0x003D] = a;
    
    a = ram[0x000E];
    if ((int8_t)a < 0) { // bmi
        a = 0;
        c->x = saved_x; // plx
        return a;
    }
    ram[0x003E] = a;
    
    uint16_t x = read16(ram, 0x003D); // ldx $3d
    a = ram[0x15C71 + x]; // lda $7f5c71,x
    ram[0x003D] = a; // sta $3d
    ram[0x003E] = 0; // stz $3e
    
    // asl $3d / rol $3e
    uint16_t val = read16(ram, 0x003D);
    val = (val << 1) & 0xFFFF; // 16-bit shift
    write16(ram, 0x003D, val);
    
    x = read16(ram, 0x003D); // ldx $3d
    a = ram[0x0EDB + x]; // lda $0edb,x
    a &= 0x83; // and #$83
    
    c->x = saved_x; // plx
    return a;
}