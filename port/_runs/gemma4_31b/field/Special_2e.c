#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: Clears a specific flag/value in WRAM and jumps to the VBlank event waiter.
static void Special_2e_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    ram[0x171C] = 0;             // stz $171c
    
    wait_vblank_event_emu(snes);  // jmp WaitVblankEvent (delegated)
}

// PITFALLS: None applicable for this trivial routine.
// HELPERS: wait_vblank_event_emu(snes) — delegates WaitVblankEvent @ $E35B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x171C=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::Special_2e ($C8:B2)