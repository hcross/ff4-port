#include "snes/snes.h"

// Logic:
// This routine calculates a memory offset based on an index Y and adds it to a base address.
// 1. Loads a 16-bit word from table _00df33[y] and adds 0xEF to it, storing in $0C.
// 2. Loads a 16-bit word from table _00df43[y] and adds 0xF1 to it, storing in $0E.
// 3. Calculates a Y-based offset: ((Y & 7) >> 1) * 16 + 0x0144.
// 4. Sets constants in $8F and $91.
static void _00df93_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // Entry: Y is 16-bit (longi). A is forced to 16-bit via longa.
    uint16_t y = cpu->y;

    // lda _00df33,y / clc / adc $ef / sta $0c
    // Table _00df33 is at ROM/RAM offset 0xDF33.
    uint16_t val1 = read16(ram, 0xDF33 + y);
    uint16_t res1 = val1 + 0xEF; // adc $ef
    write16(ram, 0x0C, res1);

    // lda _00df43,y / clc / adc $f1 / sta $0e
    uint16_t val2 = read16(ram, 0xDF43 + y);
    uint16_t res2 = val2 + 0xF1; // adc $f1
    write16(ram, 0x0E, res2);

    // tya / and #$0007 / lsr / and #$00ff / asl4 (which is << 4)
    uint16_t calc = y & 0x0007;
    calc >>= 1;
    calc &= 0x00FF;
    calc <<= 4;
    
    // clc / adc #$0144 / tay
    calc += 0x0144;
    cpu->y = calc;

    // lda #0 / shorta / lda #$18 / sta $91
    // The 'lda #0' is effectively overwritten by the next 'lda #$18' after shorta
    ram[0x91] = 0x18;
    
    // lda #$a8 / sta $8f
    ram[0x8F] = 0xA8;
}

// PITFALLS: 6 (Mode A transitions: longa for 16-bit additions, shorta for 8-bit stores),
// 7 (Arithmetic truncation: results are treated as 16-bit since longa is active)
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=16-bits
//   inputs_ram:  0xDF33=2, 0xDF43=2 (plus Y offset)
//   output_ram:  0x0C=2, 0x0E=2, 0x8F=1, 0x91=1
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00df93 ($DF:93)