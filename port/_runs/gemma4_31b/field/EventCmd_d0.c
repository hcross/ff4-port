#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$EE, DP=0
// Logic: Toggles the value at $E3 (XOR 1) and then jumps to WaitVblankEvent.
// Note: $E3 is accessed via DP=0, meaning absolute address $00:00E3 if DB=0, 
// but here it is used in the context of the field event system. 
// In the SNES memory map for this game, these low DP addresses often 
// map to specific hardware registers or a dedicated scratch area.
static void EventCmd_d0_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $e3 / eor #$01 / sta $e3
    ram[0xE3] ^= 0x01;

    // jmp WaitVblankEvent
    wait_vblank_event_emu(snes);
}

// PITFALLS: None relevant for this simple XOR and Jump sequence.
// HELPERS: wait_vblank_event_emu(snes) — delegates WaitVblankEvent @$EE:35B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00E3=1
//   output_ram:  0x00E3=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEE
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_d0 ($EE:1C)