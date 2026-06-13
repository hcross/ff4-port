#include "snes/snes.h"

// Rand: calls RandXA(A=0xFF, X=0) to generate a random byte 0-255.
// clr_ax (tdc/tax) sets A=X=DP(=0), then lda #$ff makes A_low=0xFF.
static void Rand_c(Snes *snes) {
    Cpu *c = snes->cpu;
    c->a = 0x00FF;   // tdc(A=0 from DP=0) then lda #$ff (8-bit, B preserved=0)
    c->x = 0x0000;   // tax after tdc
    c->z = false;    // flags from lda #$ff: non-zero
    c->n = true;     // bit 7 set
    randxa_emu(snes);
}

// PITFALLS: 5 (clr_ax is tdc/tax not lda#0/ldx#0; with DP=0 result is identical)
// HELPERS: randxa_emu(snes) — delegates RandXA @ $85:8379
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  (none)
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: battle::Rand ($85:93)