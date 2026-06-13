#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x7E, DP=0
// Input: X = current character index (16-bit)
// Output: X = next character index (X + 0x40)
static void NextChar_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    cpu->mf = false;              // longa (.a16)
    cpu->a = cpu->x;              // txa (16-bit)
    cpu->c = false;               // clc
    cpu->a = (uint16_t)(cpu->a + 0x40); // adc #$0040 (16-bit)
    cpu->x = cpu->a;              // tax (16-bit)
    cpu->a = 0;                   // lda #0 (8-bit after shorta)
    cpu->mf = true;               // shorta (.a8)
}

// PITFALLS: 6 (mode A 16-bit during the arithmetic, then 8-bit at end)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::NextChar ($E7:B8)