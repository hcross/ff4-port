#include "snes/snes.h"

// Purpose: Resets a specific map stack state and triggers a map stack load,
// then transitions to the Vblank event wait loop.
static void Special_0b_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // ldx #0 / stx $172c
    // Note: X is 16-bit by convention in field module for absolute stores
    write16(ram, 0x172C, 0);

    // jsr LoadMapStack
    load_map_stack_emu(snes);

    // jmp WaitVblankEvent
    // This is a tail-call jump. In a native C reimplementation, we call the
    // function. Since WaitVblankEvent is a loop/event handler, it is delegated.
    wait_vblank_event_emu(snes);
}

// PITFALLS: None relevant for this routine (straight-line execution).
// HELPERS: load_map_stack_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram: 0x172C=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_0b ($D3:61)