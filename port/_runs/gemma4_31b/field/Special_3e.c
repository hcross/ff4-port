#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: Triggers a song fade-in then jumps to the Vblank event handler.
// Note: Since the routine ends in a 'jmp', it does not return to the caller.
static void Special_3e_c(Snes *snes) {
    fade_in_song_emu(snes);       // jsr FadeInSong
    wait_vblank_event_emu(snes);  // jmp WaitVblankEvent
}

// PITFALLS: None applicable.
// HELPERS: 
//   fade_in_song_emu(snes)    - delegates FadeInSong @ $C5:26
//   wait_vblank_event_emu(snes) - delegates WaitVblankEvent @ $E3:5B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Routine ends in jmp, not rts; standard parity harness return check will fail)

// REVERSED_FUNCTION: field::Special_3e ($C5:20)