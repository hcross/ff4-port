#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E8, DP=0
// This is a trivial wrapper that zeros the accumulator and jumps
// to a shared initialization routine.
static uint16_t InitMainCharEquip_c(Snes *snes) {
    // lda #0
    snes->cpu->a = 0;
    snes->cpu->z = true;
    snes->cpu->n = false;

    // jmp _e8f6 (delegated to the target address)
    return _e8f6_emu(snes);
}

// PITFALLS: None. Simple register load and jump.
// HELPERS: _e8f6_emu(snes) - delegates jump target at $E8:F6
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE8
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::InitMainCharEquip ($E8:E6)