#include "snes/snes.h"

// This routine reads the next byte from the event stream and uses it to
// set an event switch (flag), then waits for the next Vblank to proceed.
static void EventCmd_f2_c(Snes *snes) {
    // The routine is a sequence of calls; results propagate via 
    // A register and WRAM between emulated functions.
    get_next_event_byte_emu(snes);
    set_event_switch_emu(snes);
    wait_vblank_event_emu(snes);
}

// PITFALLS: None. (Linear execution path, no arithmetic or branching).
// HELPERS: 
//   get_next_event_byte_emu(snes) - delegates GetNextEventByte @ $E553
//   set_event_switch_emu(snes)   - delegates SetEventSwitch @ $F3BB
//   wait_vblank_event_emu(snes)   - delegates WaitVblankEvent @ $E35B

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (Routines chain through emulated state, no single output RAM)

// REVERSED_FUNCTION: field::EventCmd_f2 ($F2:09)