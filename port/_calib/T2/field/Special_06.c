#include "snes/snes.h"

// This function sets $91 to #$39 and jumps to _d9e5.
// Since there are no conditionals, no flags need to be simulated.
// Entry mode assumptions: A/X can be any size (not used).
static void Special_06_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x91] = 0x39;
    // lda #$c0 / jmp _d9e5
    // We don't translate _d9e5 inline, so we simulate the jump
    // by directly setting up the state it would expect.
    // However, since we're not given the body of _d9e5, we delegate.
    // But this function doesn't actually "call" _d9e5, it jumps to it,
    // so we must emulate from _d9e5's address.
    //
    // But per the spec, we translate this function, not delegate.
    // So we just set up the jump manually.
    // The jump target is _d9e5, which is not provided.
    // Since we are to translate this function, and it just jumps,
    // we can't proceed without _d9e5's code.
    // However, the task is to translate Special_06, which just sets $91 and jumps.
    // So we simulate the jump by calling the emulator at _d9e5.
    Cpu *cpu = snes->cpu;
    cpu->db = 0x7E; // standard DB for most routines
    cpu->dp = 0;    // standard DP
    cpu->a = 0xC0;  // lda #$c0
    // Jump to _d9e5 by running emulation from that address.
    run_emulated_func(snes, 0xD9E5u);
}

// PITFALLS: 1 (DB must be set for emulator calls)
// HELPERS: run_emulated_func (used for jump target)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x91=1
//   entry_mode:  mf=auto, xf=auto, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_06 ($D9:D6)