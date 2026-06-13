#include "snes/snes.h"

// Logic:
// Sequence of synchronization and event processing:
// 1. Wait for a short Vblank period.
// 2. Execute the routine at $00:EDF6.
// 3. Jump to the main Vblank event handler loop.
static void EventCmd_d6_c(Snes *snes) {
    // Note: This function ends in a 'jmp' to WaitVblankEvent,
    // meaning it does not return to its caller.
    wait_vblank_short_emu(snes);
    _00edf6_emu(snes);
    wait_vblank_event_emu(snes);
}

// PITFALLS: None. Routine is a simple sequence of calls/jump.
// HELPERS: wait_vblank_short_emu(snes), _00edf6_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xED
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Function ends in JMP, not RTS; parity must track PC flow)

// REVERSED_FUNCTION: field::EventCmd_d6 ($ED:ED)