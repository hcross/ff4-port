#include "snes/snes.h"

static const uint8_t kDisableCmdStatusTbl[] = {
    0x00, 0x00,  // $00: fight
    0x00, 0x00,  // $01: item
    ...
};