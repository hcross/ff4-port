#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$8A (Field), DP=0
// Logic:
//   1. Set A to 0.
//   2. Call FadeOut (with A=0).
//   3. Call ScreenOff.
static void FadeOutMenu_c(Snes *snes) {
    // lda #0
    snes->cpu->a = 0;
    
    // jsr FadeOut
    fade_out_emu(snes);
    
    // jsr ScreenOff
    screen_off_emu(snes);
}

// PITFALLS: None
// HELPERS: fade_out_emu(snes), screen_off_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x8A
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::FadeOutMenu ($8A:08)