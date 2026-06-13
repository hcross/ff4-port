#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$EA, DP=0
// Logic:
//   If ram[$80] is 0: call FadeIn($07) then jump to WaitVblankEvent
//   If ram[$80] != 0: call FadeOut($07) then jump to WaitVblankEvent
static void EventCmd_da_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0x80] == 0) {                 // lda $80 / bne @ea58 (not taken)
        snes->cpu->a = 0x07;              // lda #$07
        fade_in_emu(snes);                // jsr FadeIn
        wait_vblank_event_emu(snes);      // jmp WaitVblankEvent
    } else {
        snes->cpu->a = 0x07;              // lda #$07
        fade_out_emu(snes);               // jsr FadeOut
        wait_vblank_event_emu(snes);      // jmp WaitVblankEvent
    }
}

// PITFALLS: None relevant for this routine (no arithmetic truncation or 
// complex flag dependencies).
// HELPERS: fade_in_emu(snes), fade_out_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x80=1
//   output_ram: none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEA
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (routine ends in a jump to another function, not RTS)

// REVERSED_FUNCTION: field::EventCmd_da ($EA:4C)