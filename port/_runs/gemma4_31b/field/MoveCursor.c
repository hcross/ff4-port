#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0xB0, DP=0
// Logic:
//   Plays a cursor sound effect, then waits for 7 Vblank intervals
//   using a decrementing loop in RAM address $89.
static void MoveCursor_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    play_cursor_sfx_emu(snes); // jsr PlayCursorSfx

    ram[0x89] = 0x07;          // lda #$07 / sta $89

    do {                        // @b084 loop
        wait_vblank_long_emu(snes); // jsr WaitVblankLong
        ram[0x89]--;               // dec $89
    } while (ram[0x89] != 0);      // bne @b084
}

// PITFALLS: None (straightforward loop/calls)
// HELPERS: play_cursor_sfx_emu(snes), wait_vblank_long_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x0089=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::MoveCursor ($B0:7D)