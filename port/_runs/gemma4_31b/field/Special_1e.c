#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$D8, DP=0
// Purpose: Set specific event parameters in RAM ($0AD4-$0AD5) 
// and trigger a system event via _00d7f6, then wait for VBlank.
static void Special_1e_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x0AD4] = 0x70;
    ram[0x0AD5] = 0x10;

    _00d7f6_emu(snes);        // jsr _00d7f6 (delegated)
    wait_vblank_event_emu(snes); // jmp WaitVblankEvent (delegated)
}

// PITFALLS: None applicable.
// HELPERS: _00d7f6_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0AD4=1, 0x0AD5=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xD8
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_1e ($D8:31)