#include "snes/snes.h"

// This routine copies 16 bytes from a fixed ROM/RAM address (f:_14fae6) 
// to a DP-relative address ($0300).
// Entry mode: A 8-bit (mf=1), X 8-bit (xf=1) based on the loop counter #$10.
static void _00a531_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // The source address f:_14fae6 refers to a specific location.
    // In the context of the FF4 disassembly, f: labels usually refer to ROM or 
    // a fixed memory map. Here we treat it as a memory access.
    // Note: 0x14FAE6 is likely an absolute address in the expanded memory map.
    
    for (uint8_t x = 0; x < 0x10; x++) {
        // ASM: lda f:_14fae6,x / sta $0300,x
        // Source address is 0x14FAE6 + x
        // Destination is DP:0300 + x. Since DP is usually 0 in this module:
        ram[0x0300 + x] = snes->ram[0x14FAE6 + x]; // Pitfall: Ensure source address is mapped correctly in harness
    }
}

// PITFALLS: None specifically triggered, but used 8-bit logic for x and a as per loop limit $10.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x14FAE6=16 (bytes)
//   output_ram:  0x0300=16 (bytes)
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::_00a531 ($A5:31)