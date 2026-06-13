#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Calculates an offset for event switches.
// Stores the value of (A >> 3) into a 16-bit word at 0x3D,
// and returns (A & 0x07) in register Y.
static void GetEventSwitchOffset_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a_val = (uint8_t)snes->cpu->a;

    // pha / lsr3 / sta $3d / stz $3e
    // Pitfall 7: Ensure result of shift is treated as 8-bit
    uint8_t shifted = (uint8_t)(a_val >> 3);
    ram[0x3D] = shifted;
    ram[0x3E] = 0;

    // pla / and #$07 / tay
    snes->cpu->y = (uint16_t)(a_val & 0x07);
}

// PITFALLS: 7 (Shift logic truncated to uint8_t to match 8-bit A mode)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x3D=1, 0x3E=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   output_reg:  y=8
// REVERSED_FUNCTION: field::GetEventSwitchOffset ($F4:07)