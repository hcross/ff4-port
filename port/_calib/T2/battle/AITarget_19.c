#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: no input registers required
// Logic: set A=0 and jump to TargetMonsterTypeAll
static void AITarget_19_c(Snes *snes) {
    // lda #0
    snes->cpu->a = 0;
    snes->cpu->z = true;   // Z flag set because A == 0
    snes->cpu->n = false;  // N flag clear because A >= 0
    // jmp TargetMonsterTypeAll
    target_monster_type_all_emu(snes);
}

// PITFALLS: 1 (DB must be $7E for battle module), 2 (Z/N flags set to match lda #0)
// HELPERS: target_monster_type_all_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=true, n=false
// REVERSED_FUNCTION: battle::AITarget_19 ($B9:38)