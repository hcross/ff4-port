#include "snes/snes.h"

// This function plays a sound effect and waits for vblank.
// It reads the next event byte to determine which SFX to play.
static void EventCmd_fb_c(Snes *snes) {
    uint8_t sfx = get_next_event_byte_emu(snes);  // jsr GetNextEventByte
    play_sfx_emu(snes, sfx);                      // jsr PlaySfx
    wait_vblank_event_emu(snes);                  // jmp WaitVblankEvent
}

// PITFALLS: 1 (DB must be $7E for field code), 2 (Z/N flags not relevant here)
// HELPERS: get_next_event_byte_emu, play_sfx_emu, wait_vblank_event_emu
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_fb ($E7:F7)