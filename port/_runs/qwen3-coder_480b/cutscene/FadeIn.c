#include "snes/snes.h"

// FadeIn performs a fade-in effect by waiting for VBlank 16 times,
// incrementing a counter at $3303 each time, until it reaches 15.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
static void FadeIn_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x9A] = 0;

loop_d71a:;
    wait_vblank_emu(snes);              // jsr WaitVBlank
    ram[0x9A]++;                        // inc $9a
    uint8_t a = ram[0x9A];              // lda $9a
    a &= 0x0F;                          // and #$0f
    if (a != 0) goto loop_d71a;         // bne @d71a

    ram[0x3303]++;                      // inc $3303
    a = ram[0x3303];                    // lda $3303
    if (a != 0x0F) goto loop_d71a;      // cmp #$0f / bne @d71a
    // rts
}

// PITFALLS: 1 (DB=$7E required for stz/lda/sta $9A and $3303)
// HELPERS: wait_vblank_emu(snes) — delegates WaitVBlank @ $00:DF41
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x9A=1, 0x3303=1
//   output_ram:  0x3303=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::FadeIn ($D7:18)