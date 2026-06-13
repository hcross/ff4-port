#include "snes/snes.h"

// This routine is actually a data table (constant words) rather than executable code.
// In the SNES context, this is a palette animation sequence for monsters in caves.
// Since it is purely data, the "translation" is a function that returns a pointer 
// to this data or a helper that mimics the memory access to this region.
// However, to follow the parity harness pattern for data-only labels, 
// we provide a function that returns the value at the given offset 
// from the base of this table in the ROM.

static uint16_t MonsterCaveAnimPal_get(Snes *snes, int index) {
    // The data resides in ROM bank $FB at offset $00C6.
    // The table consists of 32 words (64 bytes).
    // Index is word-based (0-31).
    
    // Based on the ASM source:
    static const uint16_t table[] = {
        0x00D0, 0x00AE, 0x008C, 0x006A, 0x0048, 0x006A, 0x008C, 0x00AE,
        0x004C, 0x004A, 0x0048, 0x0026, 0x0024, 0x0026, 0x0028, 0x004A,
        0x000A, 0x0008, 0x0006, 0x0004, 0x0002, 0x0004, 0x0006, 0x0008,
        0x0154, 0x0132, 0x0110, 0x00EE, 0x00CC, 0x00EE, 0x0110, 0x0132
    };

    if (index < 0 || index >= 32) return 0;
    return table[index];
}

// PITFALLS: None. This is a static data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Data table, no executable logic)

// REVERSED_FUNCTION: field::MonsterCaveAnimPal ($FB:C6)