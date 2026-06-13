#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$F5, DP=0
// Purpose: Increments a zoom multiplier value by 8, then divides the result by 4
//          to update a 16-bit value at $43-$44, returning the result in X.
//
// Logic:
//   1. Clear $3D
//   2. temp = ram[0x40] + 8
//   3. write temp to $40 and $44
//   4. clear $43
//   5. Shift Right (div 2) $44:$43
//   6. Shift Right (div 4) $44:$43
//   7. Load result from $43 into X
static void IncZoomMult_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    ram[0x3D] = 0;
    
    // a = ram[0x40] + 8
    uint8_t val = (uint8_t)(ram[0x40] + 8); // Pitfall 7: Truncate to 8-bit
    
    ram[0x40] = val;
    ram[0x44] = val;
    ram[0x43] = 0;
    
    // LSR $44 / ROR $43 (Shift 16-bit value in $43-$44 right by 1)
    uint16_t zoom = read16(ram, 0x43);
    zoom >>= 1;
    write16(ram, 0x43, zoom);
    
    // LSR $44 / ROR $43 (Shift 16-bit value in $43-$44 right by 1 again)
    zoom = read16(ram, 0x43);
    zoom >>= 1;
    write16(ram, 0x43, zoom);
    
    // ldx $43 (Since X is 16-bit, this loads the word at $43)
    snes->cpu->x = read16(ram, 0x43);
}

// PITFALLS: 7 (8-bit addition truncated), 1 (DB=$F5)
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x40=1
//   output_ram:  0x43=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xF5
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::IncZoomMult ($F5:18)