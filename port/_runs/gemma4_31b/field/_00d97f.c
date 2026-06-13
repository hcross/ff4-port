#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$00, DP=0
// Logic:
//   Reads a value from ram[$22].
//   If value is < 0x00F0 or >= 0x3FF0, it writes the value from ram[$22] to ram[$0301 + Y].
//   If value is between 0x00F0 and 0x3FEF inclusive, it writes 0xF0 to ram[$0301 + Y].
// Note: Since the output target is ram[$0301,y], we assume Y is 16-bit.
static void _00d97f_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t y = snes->cpu->y;

    uint16_t x = read16(ram, 0x22); // ldx $22 (X is 16-bit)

    if (x >= 0x3FF0) {               // cpx #$3ff0 / bcs @d993
        ram[0x0301 + y] = (uint8_t)x; // lda $22 / sta $0301,y (truncates to 8-bit)
        return;
    }

    if (x < 0x00F0) {               // cpx #$00f0 / bcc @d993
        ram[0x0301 + y] = (uint8_t)x; // lda $22 / sta $0301,y (truncates to 8-bit)
        return;
    }

    // Value is within [0x00F0, 0x3FEF]
    ram[0x0301 + y] = 0xF0;         // lda #$f0 / sta $0301,y
}

// PITFALLS: 3 (CMP/BCS semantics: bcs branches when X >= #$3FF0), 
// 6 (Mode A/X: X is 16-bit, A is 8-bit).
// HELPERS: read16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=16bits
//   inputs_ram:  0x22=2, 0x0301=1 (Y-offset)
//   output_ram:  0x0301=1 (Y-offset)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00d97f ($D9:7F)