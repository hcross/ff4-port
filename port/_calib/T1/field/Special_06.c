#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$D9, DP=0
// Logic:
//   sets ram[0x91] to 0x39
//   sets cpu->a to 0xC0
//   jumps to _d9e5 (delegated as it is a jump to a label within the same/nearby logic)
static void Special_06_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    ram[0x91] = 0x39;        // lda #$39 / sta $91
    cpu->a = 0xC0;           // lda #$c0
    
    // The routine ends with 'jmp _d9e5'. 
    // In a native C reimplementation, we delegate the target of the jump
    // to ensure the emulator's PC and state transition correctly.
    _d9e5_emu(snes);
}

// PITFALLS: None.
// HELPERS: _d9e5_emu(snes) — delegates jump target @ $D9:E5
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0091=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xD9
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_06 ($D9:D6)