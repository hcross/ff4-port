#include "snes/snes.h"

// This is a data table lookup routine. In the original ASM, it is a 
// sequence of bytes. The C implementation provides a helper to 
// retrieve the shop type based on an index.
static uint8_t get_shop_type(Snes *snes, uint8_t index) {
    // The table is located at $FB:A6 in the ROM.
    // Since snes->ram only covers WRAM ($7E:0000-$7F:FFFF), 
    // we access the ROM data.
    static const uint8_t shop_type_tbl[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
        1, 1, 2, 2, 2, 2, 0, 1, 2, 2, 0, 1, 0, 1, 2, 2
    };

    if (index >= (sizeof(shop_type_tbl))) {
        return 0; // Out of bounds safety
    }
    return shop_type_tbl[index];
}

// PITFALLS: None. This is a static data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFB
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: field::ShopTypeTbl ($FB:A6)