#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$F3, DP=0
// Logic:
//   1. Call GetEventSwitchOffset to determine the bit-index for the switch (returned in Y).
//   2. Construct a bitmask by shifting 1 left until the number of shifts matches Y.
//   3. Load a base address index from ram[0x3D].
//   4. Perform an OR operation on the byte at address (0x1280 + index) with the calculated bitmask.
static void SetEventSwitch_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // GetEventSwitchOffset returns the bit index in Y
    get_event_switch_offset_emu(snes);
    uint16_t y = snes->cpu->y;

    uint8_t mask = 0x01;
    // Loop @f3c0: Construct bitmask by shifting left Y times
    while (y > 0) {
        mask = (uint8_t)(mask << 1); // asl A (Pitfall 7: truncate to 8-bit)
        y--;                         // dey
    }

    uint8_t index = ram[0x3D];       // ldx $3d
    uint16_t addr = 0x1280 + index;   // ora $1280,x (Absolute + X index)
    
    // Memory access is relative to DB. 
    // Since the target is 0x1280, we must ensure the access is handled correctly.
    // In the original SNES memory map, 0x1280 is in the low memory region.
    ram[addr] |= mask;                // sta $1280,x
}

// PITFALLS: 7 (asl A in 8-bit mode must be truncated to uint8_t to ensure carry/overflow 
// behave as in 65816, although here it simply builds a bitmask).
// HELPERS: get_event_switch_offset_emu(snes) — delegates GetEventSwitchOffset @ $F3:F407
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3D=1
//   output_ram:  0x1280-0x12FF=1 (Depending on value of ram[0x3D])
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xF3
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::SetEventSwitch ($F3:BB)