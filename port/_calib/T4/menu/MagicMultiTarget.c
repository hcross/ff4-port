#include "snes/snes.h"

// ROM data table at $FF:F2 — multi-target flags for 14 magic spells.
// Accessed by menu code via absolute indexed addressing.
// 1 = spell supports multi-target selection, 0 = single-target only.
static const uint8_t kMagicMultiTarget[14] = {
    1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1
};