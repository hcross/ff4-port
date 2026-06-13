#include "snes/snes.h"

// This function sets $28 to #$20 then jumps to _e2ed.
// Since _e2ed is not yet translated, we delegate to it.
static void GetOtherPlanetTile_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x28] = 0x20;
    // Jump to _e2ed (branch)
    // A 8-bit, X 16-bit, DB=$7E, DP=0 are assumed based on typical cutscene context
    Cpu *c = snes->cpu;
    c->db = 0x7E;
    c->dp = 0;
    c->mf = true;   // A 8-bit
    c->xf = false;  // X/Y 16-bit
    run_emulated_func(snes, 0xE2EDu);
}

// PITFALLS: 1 (DB must be set to $7E for correct RAM writes)
// HELPERS: run_emulated_func (to execute _e2ed)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x28=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::GetOtherPlanetTile ($E2:DC)