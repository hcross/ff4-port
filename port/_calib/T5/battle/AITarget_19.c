#include "snes/snes.h"

// AITarget_19: tail-call to TargetMonsterTypeAll with A=0.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0 (inherited from battle caller).
// The routine does `lda #0` then `jmp TargetMonsterTypeAll`.
// We preserve the hidden B (high byte of A) as per 8-bit load semantics.
static void AITarget_19_c(Snes *snes) {
    Cpu *c = snes->cpu;
    // lda #0 in 8-bit mode: low byte = 0, high byte (B) unchanged.
    c->a &= 0xFF00;          // clear low byte, preserve B
    c->z = true;             // result is zero
    c->n = false;
    // Ensure battle module conventions: DB=$7E, DP=0, mf=1, xf=0
    c->db = 0x7E;
    c->dp = 0;
    c->mf = true;
    c->xf = false;
    // Tail jump to TargetMonsterTypeAll ($B9:3D)
    run_emulated_func(snes, 0x0B93D);
}

// PITFALLS: 1 (DB=$7E required for battle module), 6 (mode A 8-bit: lda #0
// loads only low byte, high byte B preserved), 9 (hidden B preserved via
// c->a &= 0xFF00).
// HELPERS: none (direct run_emulated_func for tail jump).
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none (tail-calls TargetMonsterTypeAll; effect is its output)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=true, n=false (set by lda #0)
//   CUSTOM_SPIKE: yes (tail call, no single output)
// REVERSED_FUNCTION: battle::AITarget_19 ($B9:38)