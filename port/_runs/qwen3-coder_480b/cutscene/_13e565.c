#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Input: X = offset (16-bit), used to index into $2000,X and $2080,X
// Output: $18-$19 = $2080,X (16-bit), then calls _13e58b with $2000,X as input
static void _13e565_c(Snes *snes, uint16_t x) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // Push X to stack (phx)
    cpu->sp--;
    ram[0x100 + cpu->sp] = x >> 8;
    cpu->sp--;
    ram[0x100 + cpu->sp] = x & 0xFF;

    // Set A to 16-bit mode (longa)
    cpu->mf = false;

    // lda $2080,x
    uint16_t val_2080 = read16(ram, 0x2080 + x);
    // sta $18
    write16(ram, 0x18, val_2080);

    // lda $2000,x
    uint16_t val_2000 = read16(ram, 0x2000 + x);
    // Set up for call to _13e58b: put val_2000 in A
    cpu->a = val_2000;
    _13e58b_emu(snes);

    // shorta0
    cpu->mf = true;

    // plx
    cpu->x = (ram[0x100 + cpu->sp] | (ram[0x101 + cpu->sp] << 8));
    cpu->sp += 2;
}

// PITFALLS: 1 (DB=$7E required), 6 (mode A 16-bit during longa), 4 (stack address in emulation mode)
// HELPERS: _13e58b_emu(snes) - delegates _13e58b @ $E5:8B
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x2000=2, 0x2080=2
//   output_ram:  0x0018=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13e565 ($E5:0065)