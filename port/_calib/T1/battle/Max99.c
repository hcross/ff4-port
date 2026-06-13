#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   This is a "clamp to 99" function. It compares the value in the
//   accumulator to 99. If A > 99, it loads 99 into A.
//   The value of A is returned in the accumulator.
static uint16_t Max99_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t a = (uint8_t)cpu->a;

    if (a > 99) {                        // cmp #99 / bcc @9e26 (bcs if A >= 99)
        a = 99;                          // lda #99
    }

    return (uint16_t)a;
}

// PITFALLS: 3 (CMP/BCC: bcc branches when A < 99, so we enter the 
// "clamp" block when A >= 99. Note that in this specific routine, 
// since we only load 99 if A > 99, A=99 is a no-op, making A > 99 
// the effective condition for the C implementation).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   custom_spike: yes (returns value in A register)

// REVERSED_FUNCTION: battle::Max99 ($9E:20)