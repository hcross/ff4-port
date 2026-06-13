#include "snes/snes.h"

static void AITarget_1b_c(Snes *snes) {
    Cpu *c = snes->cpu;
    c->a = 2; // lda #2
    // Need to set flags? lda #2 sets Z=0, N=0.
    c->z = false;
    c->n = false;
    // Mode: likely 8-bit A (mf=1) because #2 is small, but if 16-bit, A=0x0002.
    // Since battle code default is mf=true, we set A=2 (8-bit).
    // But the jmp is just a tail call to TargetMonsterTypeAll.
    target_monster_type_all_emu(snes);
}