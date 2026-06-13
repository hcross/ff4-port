#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: Sets A to 2 and jumps to TargetMonsterTypeAll.
// Since TargetMonsterTypeAll is a jump (not a JSR), it is treated as 
// part of the same logical flow.
static void AITarget_1b_c(Snes *snes) {
    snes->cpu->a = 2;
    target_monster_type_all_emu(snes);
}

// PITFALLS: None relevant for this short routine.
// HELPERS: target_monster_type_all_emu(snes) — delegates TargetMonsterTypeAll @ $B9:3D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: battle::AITarget_1b ($B9:69)