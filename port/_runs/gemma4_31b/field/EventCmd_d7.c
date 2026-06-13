#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E7, DP=0 (assuming field module)
// Logic:
//   If ram[$AC] == 0: increment ram[$AC] then jump to WaitVblankEvent
//   If ram[$AC] != 0: set ram[$AC] to 0 then jump to WaitVblankEvent
//
// Note: WaitVblankEvent is a jump target, executed as a delegated routine.
static void EventCmd_d7_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t val = ram[0xAC];
    if (val == 0) {                      // bne @e7dc (not taken)
        ram[0xAC]++;                     // inc $ac
    } else {                            // bne @e7dc (taken)
        ram[0xAC] = 0;                   // stz $ac
    }

    wait_vblank_event_emu(snes);         // jmp WaitVblankEvent
}

// PITFALLS: None (simple byte manipulation)
// HELPERS: wait_vblank_event_emu(snes) — delegates WaitVblankEvent @ $E7:E35B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xAC=1
//   output_ram:  0xAC=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE7
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_d7 ($E7:D3)