#include "snes/snes.h"

static uint16_t AICondTbl_c(uint8_t index) {
    switch (index) {
        case 0x00: return 0x...; // address of AICond_00
        ...
    }
}