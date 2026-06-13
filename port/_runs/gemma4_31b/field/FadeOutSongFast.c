#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0, DP=0
// Sets the sound command for fast fade-out and executes the sound driver.
static void FadeOutSongFast_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1E00] = 0x86;             // lda #$86 / sta $1e00
    ExecSound_ext_emu(snes);         // jsl ExecSound_ext (delegated)
}

// PITFALLS: 1 (DB=0 used for hardware/system area access at $1E00)
// HELPERS: ExecSound_ext_emu(snes) — delegates ExecSound_ext @ $80:0003
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1E00=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::FadeOutSongFast ($8D:49)