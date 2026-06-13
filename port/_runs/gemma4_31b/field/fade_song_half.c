#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// Logic: Sets music volume to 0x87 in the sound buffer and triggers the sound executor.
static void fade_song_half_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda #$87 / sta $1e00
    ram[0x1E00] = 0x87;

    // jsl ExecSound_ext
    ExecSound_ext_emu(snes);
}

// PITFALLS: None relevant.
// HELPERS: ExecSound_ext_emu(snes) — delegates ExecSound_ext @ $80:0003
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1E00=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::fade_song_half ($C5:16)