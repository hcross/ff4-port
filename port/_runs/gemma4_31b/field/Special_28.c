#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$17 (implied by $17xx writes), DP=0
// Logic:
//   - Sets the enterprise to be underground and visible.
//   - Sets coordinates to (102, 82) via $171D.
//   - Clears address $B7.
//   - Jumps to WaitVblankEvent.
static void Special_28_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Enterprise is underground and visible
    ram[0x171F] = 1;
    ram[0x171C] = 1;

    // Clear $B7
    ram[0xB7] = 0;

    // Set coordinates (102, 82) -> 0x5266
    write16(ram, 0x171D, 0x5266);

    // Jump to WaitVblankEvent (delegated as it is a system event handler)
    wait_vblank_event_emu(snes);
}

// PITFALLS: 1 (DB must be managed; the routine writes to $17xx range, 
// implying a data bank context distinct from battle's $7E).
// HELPERS: wait_vblank_event_emu(snes) — delegates WaitVblankEvent @ $CF:E35B
//          write16 — little-endian 16-bit accessor
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x171F=1, 0x171C=1, 0x00B7=1, 0x171D=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x17
//   entry_flags:  z=auto, n=auto
// REVERSED_FUNCTION: field::Special_28 ($CF:E3)