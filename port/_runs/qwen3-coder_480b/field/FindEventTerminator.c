#include "snes/snes.h"

// Scans forward through event script until terminator ($FF) is found.
// Updates the event script pointer to point past the terminator.
static void FindEventTerminator_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = read16(ram, 0x09D3);  // ldx $09d3
    x++;                               // inx
    for (;;) {
        uint8_t a = ram[0x0F0000 + x]; // lda f:EventScript,x
        if (a == 0xFF) break;          // cmp #$ff / beq @e56a
        x++;                           // inx
    }
    write16(ram, 0x09D3, x);           // stx $09d3
}

// PITFALLS: 8 (X is 16-bit, A is 8-bit — default field mode)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x09D3=2
//   output_ram:  0x09D3=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::FindEventTerminator ($E5:005A)