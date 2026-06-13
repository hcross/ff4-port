#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Input: ram[$7D1D] = character index (8-bit)
// Output: ram[$39] and ram[$3C] updated (both 16-bit)
// The routine calculates:
//   $39 = ($7D1D << 1) + $39
//   $3C = $7D1D + $39
static void NewLine_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t char_index = ram[0x7D1D];         // lda $7d1d
    uint16_t a = (uint16_t)char_index;        // longa (A now 16-bit)
    uint16_t saved_a = a;                     // pha
    a <<= 1;                                  // asl
    a += read16(ram, 0x39);                   // clc / adc $39
    write16(ram, 0x39, a);                    // sta $39
    a = saved_a;                              // pla
    a += read16(ram, 0x39);                   // clc / adc $39
    write16(ram, 0x3C, a);                    // sta $3c
    // clr_ay → A=Y=0 (but A will be truncated to 8-bit by shorta)
    // In this case, we just need Y=0 since A is not preserved after RTS
    snes->cpu->y = 0;                         // tay (Y 16-bit)
    // shorta (A back to 8-bit) — not needed since we're done
}

// PITFALLS: 6 (mode A promotion: routine starts in 8-bit, switches to 16-bit)
//           9 (upper byte B preservation: not relevant here since we start
//              with an 8-bit load and promote cleanly to 16-bit)
// HELPERS: none
// CONTRACT:
//   inputs_ram:  0x7D1D=1, 0x39=2
//   output_ram:  0x39=2, 0x3C=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::NewLine ($EB:24)