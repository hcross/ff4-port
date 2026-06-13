#include "snes/snes.h"

// Sets the music fade-in command (0x89) and executes the sound update.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x0, DP=0
static void FadeInSong_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1E00] = 0x89;        // lda #$89 / sta $1e00
    ExecSound_ext_emu(snes);   // jsl ExecSound_ext (delegated)
}

// PITFALLS: None.
// HELPERS: ExecSound_ext_emu(snes) — delegates ExecSound_ext @ $80:0003
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1E00=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::FadeInSong ($C5:26)