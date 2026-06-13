#include "snes/snes.h"

// This function sets A=2 and jumps to TargetMonsterTypeAll.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// No inputs from registers; A is set before jump.
// Output depends on TargetMonsterTypeAll implementation.
static void AITarget_1b_c(Snes *snes) {
    // lda #2
    snes->cpu->a = 2;
    // jmp TargetMonsterTypeAll
    target_monster_type_all_emu(snes);
}

// PITFALLS: 1 (DB must be $7E for WRAM access in TargetMonsterTypeAll)
// HELPERS: target_monster_type_all_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::AITarget_1b ($B9:69)