#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$F2 (Program Bank), DP=0
// This routine processes a specific event command (f4) by fetching the 
// next event byte and using it to set an NPC switch, then waiting for 
// the next Vblank before returning/continuing.
static void EventCmd_f4_c(Snes *snes) {
    // Fetch the next byte from the event stream
    get_next_event_byte_emu(snes);
    
    // The byte fetched by GetNextEventByte is now in A.
    // SetNPCSwitch uses this value to toggle the corresponding switch.
    set_npc_switch_emu(snes);
    
    // Final jump to WaitVblankEvent.
    // Since it's a jump (jmp) and not a call (jsr), we effectively
    // transfer control to that routine's implementation.
    wait_vblank_event_emu(snes);
}

// PITFALLS: None relevant for this linear call chain.
// HELPERS: 
//   get_next_event_byte_emu(snes)  — delegates GetNextEventByte @ $E5:53
//   set_npc_switch_emu(snes)       — delegates SetNPCSwitch @ $F3:D3
//   wait_vblank_event_emu(snes)    — delegates WaitVblankEvent @ $E3:5B

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xF2
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (Linear flow with delegated calls; parity depends on sub-routine side effects)

// REVERSED_FUNCTION: field::EventCmd_f4 ($F2:1B)