#include "snes/snes.h"

// This function fades out the current song slowly and then waits for a VBlank event.
// It delegates both operations to external routines.
static void EventCmd_d8_c(Snes *snes) {
    fade_out_song_slow_emu(snes);  // jsr FadeOutSongSlow
    wait_vblank_event_emu(snes);    // jmp WaitVblankEvent (tail call)
}

// PITFALLS: 1 (DB must be $7E for WRAM access if these used absolute addressing)
// HELPERS: fade_out_song_slow_emu, wait_vblank_event_emu
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=auto, xf=auto, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_d8 ($E6:0013)