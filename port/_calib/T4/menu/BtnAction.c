#include "snes/snes.h"

static const uint16_t btn_action_table[4] = {0x0000, 0x0080, 0x8000, 0x0040};

static const uint16_t* BtnAction_c(Snes *snes) {
    return btn_action_table;
}