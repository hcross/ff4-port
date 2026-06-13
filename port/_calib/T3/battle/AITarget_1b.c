#include "snes/snes.h"

// Trampoline: loads monster-type argument 2 and tail-calls TargetMonsterTypeAll.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
static void AITarget_1b_c(Snes *snes) {
    Cpu *c = snes->cpu;
    c->a = 2;                // lda #2
    c->z = false;            // 2 != 0
    c->n = false;            // bit 7 clear
    target_monster_type_all_emu(snes);  // jmp TargetMonsterTypeAll
}

// PITFALLS: 1 (DB=$7E required by emu helper)
// HELPERS: target_monster_type_all_emu(snes) — delegates TargetMonsterTypeAll @ $B9:3D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: battle::AITarget_1b ($B9:69)