#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E7, DP=0
// Logic:
//   Increments the 16-bit value in X by 0x0040, clears the 8-bit
//   accumulator, and ensures the CPU returns to 8-bit mode A.
static void NextChar_c(Snes *snes) {
    Cpu *cpu = snes->cpu;

    // longa / txa / clc / adc #$0040 / tax
    // Note: we operate on cpu->x as 16-bit regardless of entry mf
    cpu->x = cpu->x + 0x0040;

    // lda #0 / shorta
    cpu->a = 0;
    cpu->mf = true; // shorta: set A to 8-bit mode
}

// PITFALLS: 6 (Mode A explicitly toggled via longa/shorta)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE7
//   entry_flags: z=auto, n=auto
//   exit_reg:    a=0, x=x+0x40
// REVERSED_FUNCTION: field::NextChar ($E7:B8)