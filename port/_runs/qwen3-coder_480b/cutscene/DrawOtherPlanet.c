#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = palette index (8-bit)
// Logic: Shifts input left (palette * 2) and stores in $52 for sprite palette.
static void DrawOtherPlanet_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t *ram = snes->ram;
    uint8_t doubled = (uint8_t)(cpu->a << 1);  // asl (8-bit mode)
    ram[0x52] = doubled;                       // sta $52
    // bra _da70 → unconditional jump to next routine
    _da70_emu(snes);
}

// PITFALLS: 6 (mode A is 8-bit, so ASL operates on 8-bit value),
//           7 (ASL truncates to 8 bits, upper bit dropped to C flag)
// HELPERS: _da70_emu(snes) — delegates _da70 @ $DA:70
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x52=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::DrawOtherPlanet ($DA:69)