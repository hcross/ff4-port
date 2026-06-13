#include "snes/snes.h"

typedef struct {
    Cpu cpu;
    Ppu ppu;
    Dma dma;
    Apu apu;
    Cart cart;
    ...
} Snes;