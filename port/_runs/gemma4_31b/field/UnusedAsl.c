#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// Note: This routine is labeled 'UnusedAsl'. It performs 5 consecutive 
// arithmetic shift left operations on the accumulator.
// Since it is a leaf routine with no memory access, output is the register state.
static uint16_t UnusedAsl_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t a = (uint8_t)cpu->a;

    // Sequence of 5 ASL instructions
    for (int i = 0; i < 5; i++) {
        // Pitfall 7: Wrap in (uint8_t) to truncate to 8 bits and preserve carry
        cpu->c = (a & 0x80) != 0;
        a = (uint8_t)(a << 1);
    }

    cpu->a = a;
    return a;
}

// PITFALLS: 7 (Truncation of 8-bit shifts to prevent promotion to 16-bit)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (output is in register A)

// REVERSED_FUNCTION: field::UnusedAsl ($92:9C)