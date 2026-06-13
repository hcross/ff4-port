#include "snes/snes.h"

uint8_t prev = ram[0x04 + x];
if (raw == 0) {
    // no buttons
    ram[0x04 + x] = 0;
    ram[0x00 + x] = 0;
    ram[0x08 + x] = 0x18;
    return;
}
if (raw == prev) {
    // held unchanged
    uint8_t cnt = ram[0x08 + x];
    cnt--;
    ram[0x08 + x] = cnt;
    if (cnt == 0) {
        ram[0x08 + x] = 0x03;
        ram[0x00 + x] = prev; // repeat event
    } else {
        ram[0x00 + x] = 0;
    }
    return;
}
// new press or change
ram[0x04 + x] = raw;
ram[0x00 + x] = raw;
ram[0x08 + x] = 0x18;