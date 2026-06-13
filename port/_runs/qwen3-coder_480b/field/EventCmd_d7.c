#include "snes/snes.h"

// Toggles $ac between 0 and 1, then waits for vblank.
// If $ac is 0, it is incremented to 1.
// If $ac is non-zero, it is set to 0.
// In both cases, WaitVblankEvent is called afterwards.
static void EventCmd_d7_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t ac = ram[0xAC];
    if (ac != 0) {                    // bne @e7dc
        ram[0xAC] = 0;                // stz $ac
    } else {
        ram[0xAC] = 1;                // inc $ac (from 0)
    }
    wait_vblank_event_emu(snes);      // jmp WaitVblankEvent
}

// PITFALLS: 1 (DB must be $7E for WRAM access)
// HELPERS: wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0xAC=1
//   output_ram:  0xAC=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_d7 ($E7:D3)