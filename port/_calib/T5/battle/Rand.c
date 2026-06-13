#include "snes/snes.h"

// Rand: generate a random number via RandXA with A=0xFF, X=0.
// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0.
// The routine clears A/X, loads A with 0xFF, then delegates to RandXA.
// The result (in A) is returned directly.
static uint16_t Rand_c(Snes *snes) {
    Cpu *c = snes->cpu;
    c->dp = 0;
    c->db = 0x7E;               // Pitfall 1: battle module DB
    c->mf = true;               // Pitfall 6: A 8-bit
    c->xf = false;              // X/Y 16-bit

    // clr_ax: A=0, X=0, B=0 (Pitfall 9: full 16-bit A cleared)
    c->a = 0;
    c->x = 0;

    // lda #$ff: A low = 0xFF, B remains 0
    c->a = 0xFF;                // in 8-bit mode this sets low byte, high=0

    // Y is not touched by this routine; leave as inherited from caller.
    return randxa_emu(snes);
}

// PITFALLS: 1 (DB=$7E), 6 (mode A 8-bit, X/Y 16-bit), 9 (B cleared via clr_ax)
// HELPERS: randxa_emu(snes) — delegates RandXA @ $03:8379
// CONTRACT:
//   inputs_reg:  a=8 (0xFF), x=16 (0), y=16 (any)
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   note:        result returned in A (16-bit)
// REVERSED_FUNCTION: battle::Rand ($85:93)