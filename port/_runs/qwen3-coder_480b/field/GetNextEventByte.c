#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: X = event offset (16-bit), ram[$09D5,X] = next event byte
// Output: A = next event byte (8-bit)
static void GetNextEventByte_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    snes->cpu->x++;                     // inx
    write16(ram, 0xB3, snes->cpu->x);   // stx $b3
    snes->cpu->a = ram[0x09D5 + snes->cpu->x]; // lda $09d5,x
    // RTS — return value in A (8-bit)
}

// PITFALLS: 1 (DB=$7E required for correct RAM access),
//           8 (A 8-bit mode assumed — lda loads 1 byte)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  0x00B3=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetNextEventByte ($E5:0053)