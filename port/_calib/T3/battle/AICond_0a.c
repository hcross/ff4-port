#include "snes/snes.h"

// AI condition 0a: increment $de if target monster took HP damage
// (but not MP damage or HP restoration). Index = (target-5)*2 into $34D4 table.
static void AICond_0a_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t a = ram[0xD2];              // lda $d2
    a = (uint8_t)(a - 5);              // sec / sbc #$05 (carry set = no borrow)
    a = (uint8_t)(a << 1);             // asl — Pitfall 7: 8-bit truncation
    int x = a;                          // tax (B=0 assumed, X 16-bit)

    if ((ram[0x34D4 + x] | ram[0x34D5 + x]) == 0)  // lda/ora/beq
        return;
    if (ram[0x34D5 + x] & 0xC0)                     // lda/and/bne
        return;

    ram[0xDE]++;                         // inc $de
}

// PITFALLS: 7 (asl in 8-bit mode must truncate to uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xD2=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::AICond_0a ($BE:EC)