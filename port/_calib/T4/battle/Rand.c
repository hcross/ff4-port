#include "snes/snes.h"

static void Rand_c(Snes *snes) {
    Cpu *c = snes->cpu;
    c->dp = 0;          // battle convention
    c->db = 0x7E;       // battle convention (Pitfall 1)
    c->mf = true;       // inherited 8-bit A
    c->xf = false;      // inherited 16-bit X
    // clr_ax: tdc / tax. With DP=0, this clears A_low, A_high (B), and X.
    c->a = 0;
    c->x = 0;
    // lda #$ff
    c->a = 0xFF;
    c->z = false;
    c->n = true;
    // c->c unaffected? We don't need to set it explicitly unless RandXA depends on it.
    // Call RandXA
    rand_xa_emu(snes);
    // RandXA returns result in A (and maybe X?). Rand returns with whatever RandXA left in A.
    // No need to read back; the caller will check cpu->a.
}