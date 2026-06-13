#include "snes/snes.h"

write16(ram, 0x89, snes->cpu->x);
...
uint16_t x = read16(ram, 0x89);
x--;
write16(ram, 0x89, x);
if (x != 0) goto loop;