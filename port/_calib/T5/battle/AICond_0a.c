#include "snes/snes.h"

// AICond_0a: increment $DE if the damage-taken entry for attacker slot
// (ram[$D2] - 5) is non-zero and does not have the MP-damage/HP-restored
// flag (bits 6-7 of the high byte).
//
// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0.
// Assumes B (high byte of A) = 0 on entry so that `tax` zero-extends
// the 8-bit index to a valid 16-bit table offset.
static void AICond_0a_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // sec / lda $d2 / sbc #$05 / asl / tax
    uint8_t idx = (uint8_t)(ram[0xD2] - 5);   // sbc #$05 with C=1
    idx = (uint8_t)(idx << 1);                // asl (8-bit truncation)
    uint16_t x = (uint16_t)idx;               // tax (B=0 assumed)

    uint16_t dmg = read16(ram, 0x34D4 + x);   // lda $34d4,x / ora $34d5,x
    if (dmg == 0) return;                     // beq @bf04

    uint8_t hi = ram[0x34D5 + x];             // lda $34d5,x
    if (hi & 0xC0) return;                    // and #$c0 / bne @bf04

    ram[0xDE]++;                              // inc $de
}

// PITFALLS:
//   7 — 8-bit arithmetic truncation: sbc and asl wrapped with (uint8_t).
//   9 — Hidden B must be zero for tax to produce a correct 16-bit index;
//       documented as an entry assumption.
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xD2=1, 0x34D4..0x34D5+index=2 (table read, index derived from 0xD2)
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   Assumes B (high byte of A) = 0 on entry.
// REVERSED_FUNCTION: battle::AICond_0a ($BE:EC)