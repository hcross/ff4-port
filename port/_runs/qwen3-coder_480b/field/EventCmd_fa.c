#include "snes/snes.h"

// This function processes event command 0xFA: play a song and wait for vblank.
// It reads the next byte (song ID), plays the song, then waits for vblank.
static void EventCmd_fa_c(Snes *snes) {
    get_next_event_byte_emu(snes);  // jsr GetNextEventByte
    play_song_event_emu(snes);      // jsr PlaySongEvent
    wait_vblank_event_emu(snes);    // jmp WaitVblankEvent (tail call)
}

// PITFALLS: none (simple delegation chain, no register/flag dependencies)
// HELPERS: get_next_event_byte_emu, play_song_event_emu, wait_vblank_event_emu
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=auto, xf=auto, dp=auto, db=auto
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_fa ($E7:E1)