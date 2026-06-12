// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   1. Applies speed modifier to current accumulator value.
//   2. Checks ram[$AB] (timer duration).
//   3. If ram[$AB] is 0, increments it to 1 (minimum duration).
//   4. Jumps to SetTimerDur to finalize the timer state.
static void TimerDur_00_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Apply speed modifier (likely modifies snes->cpu->a)
    apply_speed_mod_emu(snes);

    // ldy $ab / bne @9e6e
    uint8_t duration = ram[0xAB];
    if (duration == 0) {
        ram[0xAB]++; // inc $ab (ensure min 1)
    }

    // jmp SetTimerDur (tail-call delegation)
    set_timer_dur_emu(snes);
}

// PITFALLS: 1 (DB=$7E assumed for battle module), 8 (mf=true for 8-bit memory access)
// HELPERS: apply_speed_mod_emu(snes), set_timer_dur_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0xAB=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (routine ends in a jump to another routine)

REVERSED_FUNCTION: battle::TimerDur_00 ($9E:65)