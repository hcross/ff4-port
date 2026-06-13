#include "snes/snes.h"

// Logic:
//   1. Fetch the next byte from the event stream.
//   2. Store that byte into ram[0x08FB] (used as the Item ID for GiveItem).
//   3. Execute the GiveItem routine.
//   4. Jump to WaitVblankEvent to synchronize with the screen refresh.
static void EventCmd_e0_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // GetNextEventByte returns the next byte in the A register
    uint8_t item_id = (uint8_t)get_next_event_byte_emu(snes);
    
    ram[0x08FB] = item_id;

    give_item_emu(snes);

    // WaitVblankEvent is a tail-call (jmp), effectively ending this sequence
    wait_vblank_event_emu(snes);
}

// PITFALLS: None relevant for this routine (linear flow).
// HELPERS: 
//   get_next_event_byte_emu(snes) -> returns A
//   give_item_emu(snes)           -> performs item granting
//   wait_vblank_event_emu(snes)   -> syncs with vblank
//
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x08FB=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_e0 ($EB:47)