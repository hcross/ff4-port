#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B0, DP=0
// Logic: Load system sound effect ID $11 into RAM address $1E00 
// and execute the sound sequence via ExecSound_ext.
static void PlayCursorSfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1E00] = 0x11;             // lda #$11 / sta $1e00
    ExecSound_ext_emu(snes);       // jsl ExecSound_ext (delegated)
}

// PITFALLS: None.
// HELPERS: ExecSound_ext_emu(snes) — delegates ExecSound_ext @ 00:8003
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1E00=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB0
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::PlayCursorSfx ($B0:73)