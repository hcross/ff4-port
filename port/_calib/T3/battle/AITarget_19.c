#include "snes/snes.h"

// AITarget_19: sets argument A=0 and tail-calls TargetMonsterTypeAll.
// Entry mode: A 8-bit (mf=true), X 16-bit (xf=false), DB=$7E, DP=0
static void AITarget_19_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    // lda #0 (8-bit): clear low byte, preserve high byte
    cpu->a = (cpu->a & 0xFF00);  // Pitfall 9: B preserved in 8-bit lda
    cpu->z = true;
    cpu->n = false;
    // jmp TargetMonsterTypeAll (tail call)
    target_monster_type_all_emu(snes);
}

// PITFALLS: 9 (high byte of A preserved across 8-bit lda #0)
// HELPERS: target_monster_type_all_emu(snes) — delegates TargetMonsterTypeAll @ $B9:3D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  (none — function hardcodes A=0)
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: battle::AITarget_19 ($B9:38)