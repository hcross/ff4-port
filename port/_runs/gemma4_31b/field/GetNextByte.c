#include "snes/snes.h"

// Entry mode: X 16-bit (xf=0), DB=$B2 (field module typically), DP=0
// This routine manages a global byte pointer/counter in WRAM.
// It reads the current index, increments it, and writes it back.
static void GetNextByte_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // ldx $0772 (X is 16-bit)
    uint16_t x = read16(ram, 0x0772);
    
    // inx
    x++;
    
    // stx $0772
    write16(ram, 0x0772, x);
}

// PITFALLS: None relevant for this simple increment. 
// Note: Assumes xf=false (16-bit X) based on the use of a 16-bit WRAM address
// and the general field module convention for pointers.
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0772=2
//   output_ram:  0x0772=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB2
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetNextByte ($B2:B7)