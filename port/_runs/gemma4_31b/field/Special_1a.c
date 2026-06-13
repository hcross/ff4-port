#include "snes/snes.h"

// Logic:
//   Initializes two 16-bit values in WRAM (0x5E and 0x60).
//   Calls a sub-routine (_00d02f) and then jumps to WaitVblankEvent.
//   Since WaitVblankEvent is a jump (not a return), this function
//   does not return to its caller.
static void Special_1a_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // ldx #$0100 / stx $5e
    // xf is 16-bit (default field convention), DB=0x00 (Direct Page access)
    write16(ram, 0x5E, 0x0100);

    // ldx #$02e0 / stx $60
    write16(ram, 0x60, 0x02E0);

    // jsr _00d02f
    _00d02f_emu(snes);

    // jmp WaitVblankEvent
    // This is a tail-call jump. In the parity harness, we call the 
    // emulated function and return, as the harness manages the PC transition.
    wait_vblank_event_emu(snes);
}

// PITFALLS: 1 (DB=0x00 used for low memory/DP writes)
// HELPERS: _00d02f_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x5E=2, 0x60=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_1a ($D0:0F)