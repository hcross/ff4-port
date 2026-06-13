#include "snes/snes.h"

// This is a data table, not a functional routine.
// It defines a sequence of sprite indices and associated values.
// In the original ROM, this is located at $F5:D6.
// The function returns the value from the table based on an index.
static uint16_t WhirlpoolSpriteTbl_c(Snes *snes, uint8_t index) {
    static const uint8_t table[] = {
        0xA0, 0x15, 0xA2, 0x15, 0xA8, 0x15, 0xAA, 0x15, 0xA4, 0x15, 0xA6, 0x15, 0xAC, 0x15, 0xAE, 0x15,
        0xAE, 0xD5, 0xAC, 0xD5, 0xA6, 0xD5, 0xA4, 0xD5, 0xAA, 0xD5, 0xA8, 0xD5, 0xA2, 0xD5, 0xA0, 0xD5
    };

    // Based on typical 65816 table access: index is usually multiplied by entry size (2 bytes).
    if (index >= (sizeof(table) / 2)) {
        return 0;
    }

    return read16(snes->ram, 0xF5D6 + (index * 2)); // Note: This is ROM data, but harness accesses via read16 mapping
}

// Since this is purely a data block in the ASM source, the "translation" 
// is providing a way to access that data that matches the ROM layout.
// In a native C port, this would be a static array.

// PITFALLS: None (Data table)
// HELPERS: read16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xF5
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::WhirlpoolSpriteTbl ($F5:D6)