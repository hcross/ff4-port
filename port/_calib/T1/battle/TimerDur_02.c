#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9E, DP=0
// Purpose: Sets the timer duration based on the value at $3558.
// If ram[0x3558] is 0, duration is 0; otherwise, duration is 1.
// This value is then modified by speed modifiers and committed via SetTimerDur.
static void TimerDur_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t val = ram[0x3558];
    uint16_t duration;

    if (val != 0) {                  // bne @9e7a
        duration = 1;                // ldx #1
    } else {
        duration = 0;               // clr_ax (A=X=D, DP=0)
    }

    ram[0xA9] = (uint8_t)duration;   // stx $a9 (Note: DP=0, writing to $00A9)

    apply_speed_mod_emu(snes);       // jsr ApplySpeedMod
    set_timer_dur_emu(snes);        // jmp SetTimerDur
}

// PITFALLS: 5 (clr_ax is tdc/tax, effectively 0 since DP=0)
// HELPERS: apply_speed_mod_emu(snes), set_timer_dur_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3558=1
//   output_ram:  0x00A9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::TimerDur_02 ($9E:71)