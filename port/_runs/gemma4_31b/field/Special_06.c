#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$D9, DP=0
// Logic:
// This routine prepares constants in WRAM and the accumulator before 
// jumping to a shared handler at _d9e5.
static void Special_06_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x91] = 0x39;           // lda #$39 / sta $91
    snes->cpu->a = 0xC0;         // lda #$c0
    
    // Jump to _d9e5. 
    // In a translation context, a JMP to a shared block is treated 
    // as a delegation of the remaining sequence.
    _d9e5_emu(snes);             // jmp _d9e5
}

// PITFALLS: None
// HELPERS: _d9e5_emu(snes) — delegates jump target @ $D9:E5
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x91=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xD9
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_06 ($D9:D6)