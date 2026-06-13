#include "snes/snes.h"

typedef struct {
    uint8_t ctrl;      // $43x0
    uint8_t dest;      // $43x1
    uint16_t src;      // $43x2-$43x3
    uint8_t bank;      // $43x4
    uint16_t size;     // $43x5-$43x6
} DmaChannel;