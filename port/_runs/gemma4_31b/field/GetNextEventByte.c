#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E5, DP=0
// Purpose: Increments the current event index, stores it in WRAM,
// and returns the byte at (0x09D5 + X).
static void GetNextEventByte_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // inx
    cpu->x++;

    // stx $b3
    write16(ram, 0xB3, cpu->x);

    // lda $09d5,x
    // Absolute indexed addressing: DB:$09D5 + X.
    // Result is 8-bit because mf=true.
    cpu->a = ram[0x09D5 + cpu->x];
}

// PITFALLS: 1 (Direct Page access $B3 and absolute access $09D5 assume DB=$E5)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x09D5=1
//   output_ram:  0x00B3=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE5
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetNextEventByte ($E5:53)