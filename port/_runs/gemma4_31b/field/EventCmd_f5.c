#include "snes/snes.h"

// Logic:
//   This routine advances the event script pointer, resets the NPC switch,
//   and then enters a wait state for the next vertical blank.
static void EventCmd_f5_c(Snes *snes) {
    get_next_event_byte_emu(snes); // jsr GetNextEventByte
    clear_npc_switch_emu(snes);    // jsr ClearNPCSwitch
    wait_vblank_event_emu(snes);   // jmp WaitVblankEvent
}

// PITFALLS: None (linear flow of delegated calls)
// HELPERS: 
//   get_next_event_byte_emu(snes) - delegates GetNextEventByte @ $E5:53
//   clear_npc_switch_emu(snes)    - delegates ClearNPCSwitch @ $F3:A0
//   wait_vblank_event_emu(snes)   - delegates WaitVblankEvent @ $E3:5B
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::EventCmd_f5 ($F2:24)