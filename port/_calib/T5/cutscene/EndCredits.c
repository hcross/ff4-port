#include "snes/snes.h"

// EndCredits: sets up cutscene id=2, duration=0x0a20, then branches to _d66b.
// The hardware register writes (f:$000064, f:$00006a-b) are handled by the
// custom spike; this C function only sets the CPU mode and delegates the tail.
static void EndCredits_c(Snes *snes) {
    Cpu *c = snes->cpu;
    // longi, shorta
    c->mf = true;
    c->xf = false;
    // The asm prologue (php, phb, phd) is expected to be pre-applied by the
    // custom spike, so the emulated stack already contains the saved state.
    _d66b_emu(snes);
}

// PITFALLS: none
// HELPERS: _d66b_emu(snes) — delegates the continuation routine at $D6:?? (label _d66b)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=<caller>, db=<caller>, e=<caller>
//   entry_flags: z=<auto>, n=<auto>
//   CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: cutscene::EndCredits ($D6:10)