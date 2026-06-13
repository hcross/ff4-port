#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$EE, DP=0
// Purpose: Implements a short delay by waiting for a specific number of vblanks.
// Sets a counter at $79 to 2, then decrements it after each call to WaitVblankShort.
// Once the counter hits 0, it jumps to WaitVblankEvent for a final sync.
static void EventCmd_d1_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x79] = 0x02; // lda #$02 / sta $79

    do {
        ram[0xC4]++;                 // inc $c4
        wait_vblank_short_emu(snes); // jsr WaitVblankShort
        ram[0x79]--;                 // dec $79
    } while (ram[0x79] != 0);        // bne @ee29

    wait_vblank_event_emu(snes);      // jmp WaitVblankEvent
}

// PITFALLS: None relevant for this routine.
// HELPERS: wait_vblank_short_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0079=1, 0x00C4=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEE
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_d1 ($EE:25)