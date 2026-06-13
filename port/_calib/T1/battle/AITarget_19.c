#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   Sets A to 0 and jumps to TargetMonsterTypeAll to target all monsters.
static void AITarget_19_c(Snes *snes) {
    // lda #0
    snes->cpu->a = 0;
    snes->cpu->z = true;
    snes->cpu->n = false;

    // jmp TargetMonsterTypeAll
    target_monster_type_all_emu(snes);
}

// PITFALLS: None
// HELPERS: target_monster_type_all_emu(snes) — delegates TargetMonsterTypeAll @ $B9:3D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: battle::AITarget_19 ($B9:38)