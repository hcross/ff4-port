#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This function performs a 9-byte ROR (rotate right through carry) chain
// starting from ram[$02] and continuing through ram[$2F03,X] to ram[$2F0A,X].
// The carry flag is propagated through each ROR.
static void _13d89a_c(Snes *snes, uint16_t x) {
    uint8_t *ram = snes->ram;
    // First ROR on ram[$02]
    uint8_t c = snes->cpu->c; // get initial carry
    uint8_t val02 = ram[0x02];
    c = (c << 7) | (val02 & 1); // new carry is LSB of value
    ram[0x02] = (val02 >> 1) | (c & 0x80); // rotate in carry from high bit
    c = c & 1; // update carry flag for next operation

    // Chain of RORs from $2F03,x to $2F0A,x
    for (int i = 0; i < 8; i++) {
        int addr = 0x2F03 + x + i;
        uint8_t val = ram[addr];
        uint8_t new_c = val & 1;
        ram[addr] = (val >> 1) | (c << 7);
        c = new_c;
    }

    // Update final carry flag in CPU state
    snes->cpu->c = c;
}

// PITFALLS: 6 (mode A is 8-bit), 7 (arithmetic truncation not relevant here,
// but bitwise operations must be precise), 1 (DB=$7E assumed)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x02=1, 0x2F03=1, 0x2F04=1, 0x2F05=1, 0x2F06=1, 0x2F07=1, 0x2F08=1, 0x2F09=1, 0x2F0A=1
//   output_ram:  0x02=1, 0x2F03=1, 0x2F04=1, 0x2F05=1, 0x2F06=1, 0x2F07=1, 0x2F08=1, 0x2F09=1, 0x2F0A=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: c=auto
// REVERSED_FUNCTION: cutscene::_13d89a ($D8:9A)