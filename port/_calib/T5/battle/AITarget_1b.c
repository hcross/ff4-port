#include "snes/snes.h"

// AITarget_1b: tail-call to TargetMonsterTypeAll with A = 2.
// The original does `lda #2 / jmp TargetMonsterTypeAll`; we emulate the
// tail call by discarding the return address that was pushed for this
// routine, then calling TargetMonsterTypeAll as a subroutine so that its
// RTS returns directly to the original caller.
static void AITarget_1b_c(Snes *snes) {
    Cpu *cpu = snes->cpu;

    // lda #2 in 8-bit mode: only the low byte is loaded; preserve B (high byte).
    cpu->a = (cpu->a & 0xFF00) | 2;

    // Pop the return address that the simulated JSR to AITarget_1b pushed.
    // Native-mode SP is 16-bit; +2 discards the 2-byte return address.
    cpu->sp += 2;

    // Ensure data bank is WRAM (Pitfall 1).
    cpu->db = 0x7E;

    // Tail-call TargetMonsterTypeAll.  run_emulated_func will push its own
    // return address, execute the routine, and its RTS will pop that address,
    // leaving SP exactly where it was before the original JSR to AITarget_1b.
    run_emulated_func(snes, 0xB93D);
}

// PITFALLS:
//   1 (DB=$7E required before run_emulated_func)
//   9 (8-bit lda #2 preserves B; we use (cpu->a & 0xFF00) | 2)
// HELPERS:
//   run_emulated_func(snes, 0xB93D) — executes TargetMonsterTypeAll
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none   (tail call to TargetMonsterTypeAll; see CUSTOM_SPIKE)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
// CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: battle::AITarget_1b ($B9:69)