#include "snes/snes.h"

// Toggles bit 0 of ram[$E3] and jumps to WaitVblankEvent.
// No register inputs or outputs; all state in WRAM.
static void EventCmd_d0_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xE3] ^= 1;              // lda $e3 / eor #$01 / sta $e3
    wait_vblank_event_emu(snes); // jmp WaitVblankEvent
}

// PITFALLS: 1 (DB must be $7E for WRAM access)
// HELPERS: wait_vblank_event_emu(snes) — delegates WaitVblankEvent @ $EE:35B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00E3=1
//   output_ram:  0x00E3=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_d0 ($EE:001C)