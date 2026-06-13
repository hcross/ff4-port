#include "snes/snes.h"

// Logic:
//   Fades the current song by half then waits for the next VBlank event.
//   Since WaitVblankEvent is a jump (jmp), this routine does not return.
static void Special_3d_c(Snes *snes) {
    fade_song_half_emu(snes);    // jsr fade_song_half
    wait_vblank_event_emu(snes); // jmp WaitVblankEvent
}

// PITFALLS: None.
// HELPERS: fade_song_half_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Routine ends in a jump, not a return)

// REVERSED_FUNCTION: field::Special_3d ($C5:10)