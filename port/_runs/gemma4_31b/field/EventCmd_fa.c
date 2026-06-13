#include "snes/snes.h"

// Logic:
// This routine is a command handler for field events.
// It fetches the next event byte, passes it to the song event handler,
// and then jumps to the VBlank wait routine to synchronize execution.
static void EventCmd_fa_c(Snes *snes) {
    // Routine calls a sequence of event handlers.
    // Results of GetNextEventByte are passed via the accumulator to PlaySongEvent.
    get_next_event_byte_emu(snes);
    play_song_event_emu(snes);
    wait_vblank_event_emu(snes);
}

// PITFALLS: None relevant for this sequence of calls.
// HELPERS: 
//   get_next_event_byte_emu(snes) - delegates GetNextEventByte @ $E5:53
//   play_song_event_emu(snes)     - delegates PlaySongEvent @ $E7:EA
//   wait_vblank_event_emu(snes)   - delegates WaitVblankEvent @ $E3:5B
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::EventCmd_fa ($E7:E1)