#include "snes/snes.h"

// Compute damage delta (2709 - 2707), store in $a4.
// Set $a5 high bit, then jump to SetMagicStatus.
// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=$7E, DP=0
static void MagicEffect_24_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    // longa: A is 16-bit
    uint16_t a = read16(ram, 0x2709);      // lda $2709
    uint16_t subtrahend = read16(ram, 0x2707); // sbc $2707
    a = (uint16_t)(a - subtrahend);        // sec + sbc = subtract with carry
    write16(ram, 0xA4, a);                 // sta $a4

    // shorta0: A is now 8-bit, high byte cleared
    uint8_t a5 = ram[0xA5];                // lda $a5
    a5 |= 0x80;                            // ora #$80
    ram[0xA5] = a5;                        // sta $a5

    set_magic_status_emu(snes);            // jmp SetMagicStatus
}

// PITFALLS: 1 (DB=$7E), 6 (mode A 16-bit on entry), 7 (16-bit arithmetic truncation)
// HELPERS: set_magic_status_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2707=2, 0x2709=2
//   output_ram:  0xA4=2, 0xA5=1
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::MagicEffect_24 ($DD:06)