#include "snes/snes.h"

/**
 * PlayerSpriteTiles is not a routine, but a data table mapping player
 * states/directions to sprite tiles.
 * 
 * Based on the provided ASM, this is a static array of 16-bit words
 * (though encoded as bytes) where each entry represents a tile index 
 * and a property/attribute.
 */
static const uint16_t PLAYER_SPRITE_TILES[] = {
    0x2004, 0x2005, 0x2006, 0x2007,
    0x2004, 0x2005, 0x6007, 0x6006,
    0x6009, 0x6008, 0x600B, 0x600A,
    0x600D, 0x600C, 0x600F, 0x600E,
    0x2000, 0x2001, 0x2002, 0x2003,
    0x2000, 0x2001, 0x6003, 0x6002,
    0x2008, 0x2009, 0x200A, 0x200B,
    0x200C, 0x200D, 0x200E, 0x200F,
    0x2014, 0x2015, 0x2016, 0x2017,
    0x2018, 0x2019, 0x201A, 0x201B,
    0x2018, 0x2019, 0x201A, 0x201B,
    0x2018, 0x2019, 0x201A, 0x201B,
    0x2010, 0x2011, 0x2012, 0x2013,
    0x2010, 0x2011, 0x2012, 0x2013,
    0x201C, 0x201D, 0x201E, 0x201F,
    0x201C, 0x201D, 0x201E, 0x201F
};

// Note: Since this was provided as a routine to be "translated" but consists 
// entirely of .byte data, I am providing the data representation. 
// If the parity harness expects a function, this would be accessed via 
// read16(snes->ram, 0xC0C4 + (index * 2)).

// PITFALLS: None (Data table)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC0
//   entry_flags: auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::PlayerSpriteTiles ($C0:C4)