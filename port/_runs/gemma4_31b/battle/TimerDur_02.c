// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9E, DP=0
// Purpose: Determine the initial timer duration based on a flag in RAM,
// then apply speed modifications and set the final timer duration.
//
// Logic:
//   If ram[0x3558] != 0: X = 1
//   Else:              X = 0 (via clr_ax)
//   The value in X is stored to ram[0xA9], then passed to ApplySpeedMod.
static void TimerDur_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $3558 / bne @9e7a
    if (ram[0x3558] != 0) {
        // @9e7a: ldx #1
        // Note: X is 16-bit per battle module convention (xf=0)
        snes->cpu->x = 1;
    } else {
        // clr_ax (tdc / tax)
        // DP=0, so A and X are cleared
        snes->cpu->a = 0;
        snes->cpu->x = 0;
    }

    // @9e7d: stx $a9
    // X is 16-bit, so this writes 2 bytes to $A9-$AA
    write16(ram, 0xA9, (uint16_t)snes->cpu->x);

    // jsr ApplySpeedMod
    apply_speed_mod_emu(snes);

    // jmp SetTimerDur
    set_timer_dur_emu(snes);
}

// PITFALLS: 5 (clr_ax uses tdc/tax), 8 (Inherited mf=true, xf=false for battle module)
// HELPERS: apply_speed_mod_emu(snes), set_timer_dur_emu(snes), write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3558=1
//   output_ram:  0x00A9=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::TimerDur_02 ($9E:71)