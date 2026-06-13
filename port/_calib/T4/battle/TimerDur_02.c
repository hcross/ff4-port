#include "snes/snes.h"

static void <FuncName>_emu(Snes *snes /*, optional args from caller */) {
    Cpu *c = snes->cpu;
    c->dp = 0;
    c->db = 0x7E;
    c->mf = true;
    c->xf = false;
    // Set input registers if the asm reads them at entry:
    // c->a = arg1; c->x = arg2; c->y = arg3;
    // If first instruction is a conditional branch (Pitfall 2):
    // c->z = (arg1 == 0); c->n = (arg1 & 0x80) != 0;
    run_emulated_func(snes, 0x<bank><offset>u);
}