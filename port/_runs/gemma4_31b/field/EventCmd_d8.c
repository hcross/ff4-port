#include "snes/snes.h"

// This routine triggers a slow fade-out of the music and then
// synchronizes with the vertical blank before returning.
static void EventCmd_d8_c(Snes *snes) {
    fade_out_song_slow_emu(snes); // jsr FadeOutSongSlow
    wait_vblank_event_emu(snes);  // jmp WaitVblankEvent
}

// PITFALLS: None.
// HELPERS: fade_out_song_slow_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_d8 ($E6:13)