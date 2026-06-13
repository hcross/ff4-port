#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$CF, DP=0
// Purpose: Decrements a 16-bit counter at $172C by 3 and waits for Vblank.
static void Special_1d_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // ldx $172c (X is 16-bit)
    uint16_t x = read16(ram, 0x172C);
    
    // dex3: Macro/Sequence for subtracting 3 from X
    x -= 3;
    
    // stx $172c
    write16(ram, 0x172C, x);

    // jmp WaitVblankEvent (delegated)
    wait_vblank_event_emu(snes);
}

// PITFALLS: None (Straightforward 16-bit subtraction and delegation)
// HELPERS: wait_vblank_event_emu(snes) — delegates WaitVblankEvent @ $E35B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x172C=2
//   output_ram:  0x172C=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xCF
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::Special_1d ($CF:C4)