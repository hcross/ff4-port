#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$F2, DP=0
// Logic:
//   1. Advance the event script pointer to the next byte.
//   2. Reset the event switch flag.
//   3. Jump to WaitVblankEvent (which handles timing/sync).
static void EventCmd_f3_c(Snes *snes) {
    get_next_event_byte_emu(snes);  // jsr GetNextEventByte
    clear_event_switch_emu(snes);   // jsr ClearEventSwitch
    wait_vblank_event_emu(snes);    // jmp WaitVblankEvent
}

// PITFALLS: None (straight-line sequence of calls)
// HELPERS: get_next_event_byte_emu(snes), clear_event_switch_emu(snes),
//          wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xF2
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::EventCmd_f3 ($F2:12)