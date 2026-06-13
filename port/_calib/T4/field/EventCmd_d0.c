#include "snes/snes.h"

ram[DP + 0xE3] ^= 0x01;
wait_vblank_event_emu(snes);