#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: ram[$3558] = timer flag
// Output: sets $a9 and calls ApplySpeedMod, then jumps to SetTimerDur
static void TimerDur_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t flag = ram[0x3558];  // lda $3558
    uint16_t x;
    if (flag != 0) {             // bne @9e7a
        x = 1;                   // ldx #1
    } else {
        x = 0;                   // clr_ax → X = 0 (A = 0 too, but unused)
    }
    write16(ram, 0xA9, x);       // stx $a9
    apply_speed_mod_emu(snes);   // jsr ApplySpeedMod
    set_timer_dur_emu(snes);     // jmp SetTimerDur
}

// PITFALLS: 1 (DB=$7E assumed), 8 (mode A 8-bit, X 16-bit inherited)
// HELPERS: apply_speed_mod_emu, set_timer_dur_emu
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3558=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::TimerDur_02 ($9E:71)