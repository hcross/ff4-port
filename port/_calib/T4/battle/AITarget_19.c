#include "snes/snes.h"

static void AITarget_19_c(Snes *snes) {
    Cpu *c = snes->cpu;
    c->a = 0;          // lda #0
    c->z = true;       // Z flag set by lda #0
    c->n = false;      // N flag cleared by lda #0
    target_monster_type_all_emu(snes); // jmp TargetMonsterTypeAll
}