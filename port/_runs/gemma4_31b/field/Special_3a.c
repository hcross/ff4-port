#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x00, DP=0
// Logic: 
//   1. Sets specific coordinates/values at $0AD4 and $0AD5.
//   2. Calls a sub-routine (likely a screen or object update).
//   3. Jumps to WaitVblankEvent (effectively ending the current frame's processing).
static void Special_3a_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x0AD4] = 0x70;
    ram[0x0AD5] = 0x70;

    // jsr _00d7f6
    _00d7f6_emu(snes);

    // jmp WaitVblankEvent
    // Because this is a 'jmp' and not a 'jsr', we call the helper 
    // and return, as the original flow does not return here.
    wait_vblank_event_emu(snes);
}

// PITFALLS: None.
// HELPERS: _00d7f6_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x0AD4=1, 0x0AD5=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: auto
// REVERSED_FUNCTION: field::Special_3a ($D8:41)