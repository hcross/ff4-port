#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$EE, DP=0
// This routine toggles a state bit in RAM ($E3) and then jumps to a vblank wait.
// Logic:
//   ram[$E3] = ram[$E3] XOR 0x01
//   jump to WaitVblankEvent
static void EventCmd_d0_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xE3] ^= 0x01;                // lda $e3 / eor #$01 / sta $e3
    wait_vblank_event_emu(snes);     // jmp WaitVblankEvent (delegated)
}

// PITFALLS: 1 (DB=$EE used for field module access)
// HELPERS: wait_vblank_event_emu(snes) — delegates WaitVblankEvent @ $EE:E35B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00E3=1
//   output_ram:  0x00E3=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEE
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::EventCmd_d0 ($EE:1C)