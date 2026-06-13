#include "snes/snes.h"

// FadeOut performs a fade-out effect by decrementing $3303 every 16 frames.
// It waits for VBlank 16 times, then decrements the fade counter.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
static void FadeOut_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x9A] = 0;

loop_d707:
    wait_vblank_emu(snes);           // jsr WaitVBlank
    ram[0x9A]++;                     // inc $9a
    uint8_t a = ram[0x9A] & 0x0F;    // lda $9a / and #$0f
    if (a != 0) goto loop_d707;      // bne @d707

    ram[0x3303]--;                   // dec $3303
    if (ram[0x3303] != 0) goto loop_d707; // bne @d707
}

// PITFALLS: 1 (DB=$7E required for WaitVBlank), 6 (A is 8-bit)
// HELPERS: wait_vblank_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3303=1
//   output_ram:  0x3303=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::FadeOut ($D7:0005)