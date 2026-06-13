#include "snes/snes.h"

// TimerDur_02: load timer flag at $3558; if non-zero set X=1 else X=0;
// store X to $A9 (16-bit), call ApplySpeedMod, then tail-jump to SetTimerDur.
// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0.
// The routine itself sets Z/N from the lda; no caller flag contract needed.
static void TimerDur_02_c(Snes *snes) {
    Cpu *c = snes->cpu;
    uint8_t *ram = snes->ram;

    // Ensure battle module conventions (Pitfall 1, Pitfall 8)
    c->dp = 0;
    c->db = 0x7E;
    c->mf = true;
    c->xf = false;

    uint8_t val = ram[0x3558];          // lda $3558 (8-bit load)
    // lda sets Z and N based on the loaded value
    c->z = (val == 0);
    c->n = (val & 0x80) != 0;
    // A low byte = val; high byte (B) preserved from entry (Pitfall 9)
    c->a = (c->a & 0xFF00) | val;

    if (val != 0) {                     // bne @9e7a
        c->x = 1;                       // ldx #1
        // ldx #1 sets Z/N based on X (16-bit)
        c->z = false;
        c->n = false;
    } else {
        // clr_ax: tdc / tax  (DP=0 → A=0, X=0, B=0)
        c->a = 0;
        c->x = 0;
        // tdc does not modify flags; Z/N remain as set by lda (val=0)
    }

    write16(ram, 0xA9, c->x);          // stx $a9

    apply_speed_mod_emu(snes);          // jsr ApplySpeedMod
    set_timer_dur_emu(snes);            // jmp SetTimerDur (tail call)
}

// PITFALLS:
//   1 (DB=$7E required for battle subroutines)
//   8 (explicit mf/xf set to avoid caller mode pollution)
//   9 (B preserved on branch-taken path; we keep high byte of A)
// HELPERS:
//   apply_speed_mod_emu(snes)  — delegates ApplySpeedMod @ $9E:D8
//   set_timer_dur_emu(snes)    — delegates SetTimerDur   @ $9E:CF
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3558=1
//   output_ram:  none   (side effects via ApplySpeedMod + SetTimerDur)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: battle::TimerDur_02 ($9E:71)