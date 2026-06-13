#include "snes/snes.h"

// Logic:
// This routine performs a hard system reset. It disables the screen, 
// disables NMIs, resets the SPC program, and jumps to the Reset vector.
static void Special_3c_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Note: $2100, $4200, $2140 are hardware registers (I/O), 
    // not WRAM. In the LakeSnes-based harness, we write to
    // the snes->regs mapping or use the emulator's write mechanism.
    // Following the snesrev pattern for hardware access:
    
    snes->regs[0x2100] = 0x80; // Screen off
    snes->regs[0x4200] = 0x00; // Disable NMI
    snes->regs[0x2140] = 0xFF; // Reset SPC program

    reset_emu(snes);           // jmp Reset (delegated)
}

// PITFALLS: None (Straight-line I/O and jump)
// HELPERS: reset_emu(snes) — delegates Reset @ 8000
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Routine ends in a jump to Reset, preventing return)

// REVERSED_FUNCTION: field::Special_3c ($C4:FE)