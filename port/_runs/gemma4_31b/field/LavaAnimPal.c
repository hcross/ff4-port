#include "snes/snes.h"

/**
 * LavaAnimPal is a data table containing palette definitions for lava animation.
 * It consists of 4 sets of 8 16-bit color values (32 words total).
 * 
 * Note: In the original ASM, this is a data block, not an executable routine.
 * The "translation" involves providing the data in a way that the parity harness
 * can validate against the ROM content at $FA:16.
 */
static const uint16_t LAVA_ANIM_PAL_DATA[] = {
    0x63FF, 0x4BFF, 0x2F9F, 0x2F3F, 0x265F, 0x150E, 0x0095, 0x0C0E, // Set 1
    0x5BFF, 0x3F9F, 0x337F, 0x2AFF, 0x1A1E, 0x150E, 0x0074, 0x0C0E, // Set 2
    0x53FF, 0x335F, 0x2B1F, 0x2ABF, 0x11BC, 0x150E, 0x0074, 0x080E, // Set 3
    0x4BFF, 0x277F, 0x26FF, 0x267F, 0x095B, 0x150E, 0x0053, 0x080E  // Set 4
};

// Since this is data and not a function, the parity harness validates 
// that the memory at $FA:16 matches this array.
static void LavaAnimPal_c(Snes *snes) {
    // No-op: This is a data label.
}

// PITFALLS: None. (Pure data block)
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFA
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LavaAnimPal ($FA:16)