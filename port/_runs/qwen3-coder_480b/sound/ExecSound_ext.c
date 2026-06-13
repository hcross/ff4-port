#include "snes/snes.h"

// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=$7E, DP=0
// This function is a thin wrapper that calls ExecSound and returns via RTL.
// No inputs or outputs in RAM or registers — purely a control flow delegate.
static void ExecSound_ext_c(Snes *snes) {
    exec_sound_emu(snes);  // jsr ExecSound
    // RTL is handled by the caller (Cpu_runOpcode handles it after return)
}

// PITFALLS: 1 (DB=$7E required for JSR targets in battle/sound)
// HELPERS: exec_sound_emu(snes) — delegates ExecSound @ $8107
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: sound::ExecSound_ext ($80:03)