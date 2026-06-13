#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E7, DP=0
// Purpose: Event command 'fb' (sound effect). Fetches the next byte 
// from the event stream and plays it as a sound effect.
static void EventCmd_fb_c(Snes *snes) {
    // jsr GetNextEventByte: fetches the SFX ID into the accumulator
    get_next_event_byte_emu(snes);
    
    // jsr PlaySfx: plays the SFX ID currently in A
    play_sfx_emu(snes);
    
    // jmp WaitVblankEvent: synchronization point
    wait_vblank_event_emu(snes);
}

// PITFALLS: None relevant for this sequence.
// HELPERS: 
//   get_next_event_byte_emu(snes) - delegates GetNextEventByte @ $E5:53
//   play_sfx_emu(snes)            - delegates PlaySfx @ $E7:F5DF (Note: bank may be E7 or E5)
//   wait_vblank_event_emu(snes)   - delegates WaitVblankEvent @ $E3:5B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE7
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::EventCmd_fb ($E7:F7)