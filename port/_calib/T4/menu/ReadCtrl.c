#include "snes/snes.h"

uint8_t flags = ram[0x16B8] & ram[0x0040];
if (flags == 0) goto fdb4;