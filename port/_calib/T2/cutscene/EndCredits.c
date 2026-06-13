#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This function initializes cutscene state for the ending credits.
// It sets the cutscene ID to 2 and duration to $0A20 frames, then
// branches to a shared continuation routine (_d66b).
static void EndCredits_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x64] = 2;         // cutscene id
    ram[0x6A] = 0x20;      // low byte of duration
    ram[0x6B] = 0x0A;      // high byte of duration
    // fall through to _d66b (shared cutscene setup)
    _d66b_emu(snes);
}

// PITFALLS: 1 (DB must be $7E for WRAM access), 8 (A/X mode inheritance)
// HELPERS: _d66b_emu(snes) — delegates _d66b @ $D6:6B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x64=1, 0x6A=1, 0x6B=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::EndCredits ($D6:0010)