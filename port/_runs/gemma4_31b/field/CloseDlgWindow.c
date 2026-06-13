#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B3, DP=0
// Purpose: Gradually close the dialogue window by decrementing its height 
// each Vblank until it reaches 0, then hiding the window.
//
// Logic:
// 1. Wait for Vblank.
// 2. Wait until the IRQ counter ($7F) reaches 2.
// 3. Decrement window height ($DF).
// 4. If height > 0, repeat from step 1.
// 5. Once height is 0, set visibility ($EC) to 1 (hidden), wait for one last Vblank, and clear height.
static void CloseDlgWindow_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    while (1) {
        wait_vblank_long_emu(snes); // jsr WaitVblankLong

        // Wait for 2nd IRQ (Polling $7F)
        while (ram[0x7F] != 0x02) { // lda $7f / cmp #$02 / bne @b3a1
            // Spin
        }

        if (ram[0xDF] == 0) break;  // dec $df / lda $df / cmp #$00 / bne @b39e
        ram[0xDF]--;                // dec $df
    }

    ram[0xEC] = 0x01;                // lda #$01 / sta $ec (hide window)
    wait_vblank_long_emu(snes);      // jsr WaitVblankLong
    ram[0xDF] = 0;                  // stz $df
}

// PITFALLS: 7 (Arithmetic truncation: ram[0xDF]-- is treated as uint8_t)
// HELPERS: wait_vblank_long_emu(snes) — delegates WaitVblankLong @ $912D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7F=1, 0xDF=1, 0xEC=1
//   output_ram:  0xDF=1, 0xEC=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB3
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::CloseDlgWindow ($B3:9E)