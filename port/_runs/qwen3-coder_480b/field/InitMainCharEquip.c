#include "snes/snes.h"

static void InitMainCharEquip_c(Snes *snes) {
    // This function unconditionally jumps to _e8f6.
    // The accumulator is set to 0 before the jump.
    Cpu *cpu = snes->cpu;
    cpu->a = 0;
    cpu->z = true;
    cpu->n = false;
    run_emulated_func(snes, 0xE8F6);
}

// PITFALLS: 2 (Z/N flags set to reflect A=0 for potential branch at _e8f6)
// HELPERS: run_emulated_func (direct jump to _e8f6)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=true, n=false
// REVERSED_FUNCTION: field::InitMainCharEquip ($E8:E6)