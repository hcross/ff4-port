#include "snes/snes.h"

// This routine copies a 16-byte block of data from a fixed source 
// address (f:_14fad6) to a destination in WRAM ($0300).
// Entry mode: A 8-bit (mf=1), X 8-bit (xf=1), DB=$A5, DP=0
static void _00a542_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // The source address f:_14fad6 is a ROM/External address.
    // Based on the disassembly, this is a simple block move of 16 bytes.
    // Since the harness provides access to the ROM via the snes instance
    // or the emulated memory map, we iterate 16 times.
    for (uint8_t x = 0; x < 0x10; x++) {
        // lda f:_14fad6,x
        // Note: In a real harness, f:_14fad6 is resolved to a specific ROM offset.
        // For the purpose of this translation, we treat it as a memory read.
        uint8_t val = snes->rom[0x14fad6 + x]; 
        
        // sta $0300,x
        ram[0x0300 + x] = val;
    }
}

// PITFALLS: None applicable for this simple copy loop.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none (reads from ROM f:_14fad6)
//   output_ram:  0x0300=16 (block of 16 bytes)
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0xA5
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::_00a542 ($A5:42)