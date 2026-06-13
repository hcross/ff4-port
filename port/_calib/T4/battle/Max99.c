#include "snes/snes.h"

static uint8_t Max99_c(Snes *snes, uint8_t a) {
    if (a < 99) {
        return a;
    }
    return 99;
}