#include "snes/snes.h"

// This "routine" is actually a data table.
// In the C reimplementation, it is converted to a static constant array
// representing the zoom level offsets used during battle transitions.
static const uint8_t BATTLE_ZOOM_TBL[] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x15, 
    0x14, 0x13, 0x12, 0x11, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 
    0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};

// Since this is a data table, we provide a helper function to access it,
// mirroring how the 65816 would index into this address.
uint8_t get_battle_zoom_value(int index) {
    if (index < 0 || index >= (int)(sizeof(BATTLE_ZOOM_TBL) / sizeof(uint8_t))) {
        return 0;
    }
    return BATTLE_ZOOM_TBL[index];
}

// PITFALLS: None. This is a static data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Data table, not a functional routine)

// REVERSED_FUNCTION: field::BattleZoomTbl ($FB:5E)