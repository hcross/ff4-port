#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0, DP=0
// Purpose: Checks if there is a pending map stack operation; if so,
// decrements the counter and triggers a map stack load.
static void Special_0a_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // ldx $172c (X is 16-bit, load word)
    uint16_t x = read16(ram, 0x172C);

    if (x == 0) { // beq @d35e
        wait_vblank_event_emu(snes);
        return;
    }

    // dex3: This is a ca65 macro for "decrement X by 3"
    // In 65816 asm, this usually expands to: dex / dex / dex
    x -= 3;
    write16(ram, 0x172C, x); // stx $172c

    load_map_stack_emu(snes); // jsr LoadMapStack

    wait_vblank_event_emu(snes); // jmp WaitVblankEvent
}

// PITFALLS: None. Simple linear flow with 16-bit X.
// HELPERS: load_map_stack_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x172C=2
//   output_ram: 0x172C=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::Special_0a ($D3:50)