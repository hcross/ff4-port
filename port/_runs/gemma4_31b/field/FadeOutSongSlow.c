#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// Logic: Sets the sound command byte for a slow music fade-out and triggers sound execution.
static void FadeOutSongSlow_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1E00] = 0x85;           // lda #$85 / sta $1e00
    ExecSound_ext_emu(snes);     // jsl ExecSound_ext
}

// PITFALLS: None relevant for this routine (no arithmetic or conditional branches).
// HELPERS: ExecSound_ext_emu(snes) — delegates ExecSound_ext @ $80:0003
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1E00=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::FadeOutSongSlow ($8D:53)