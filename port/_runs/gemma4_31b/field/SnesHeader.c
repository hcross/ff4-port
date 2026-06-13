#include "snes/snes.h"

// The SnesHeader is not a functional routine (code) but a data structure 
// defined in the ROM header. In a C reimplementation targeting the 
// game-and-watch-retro-go-sd, this represents the static ROM identity 
// data located at $FF:C0. 
// Since this is data and not an executable routine, the "translation" 
// is a constant byte array matching the ROM's layout.
static const uint8_t SNES_HEADER[] = {
    'F', 'I', 'N', 'A', 'L', ' ', 'F', 'A', 'N', 'T', 'A', 'S', 'Y', ' ', '4', ' ', 
    ' ', ' ', ' ', ' ',                                       // "FINAL FANTASY 4      "
    0x20,                                                     // LoROM, SlowROM
    0x02,                                                     // rom + ram + sram
    0x0A,                                                     // rom size: 16 Mbit
    0x03,                                                     // sram size: 64 kbit
    0x00,                                                     // destination: japan
    0xC3,                                                     // publisher: squaresoft
    0x00,                                                     // revision number (ROM_VERSION=0)
    0x00, 0x00,                                               // checksum
    0xFF, 0xFF                                               // inverse checksum
};

// PITFALLS: None. This is a static data block, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none
// CUSTOM_SPIKE: yes (Data block, parity validated by ROM hash/checksum)

// REVERSED_FUNCTION: field::SnesHeader ($FF:C0)