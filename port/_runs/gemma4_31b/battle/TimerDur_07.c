// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Determines timer duration based on a condition flag at $3558.
// If $3558 is non-zero, it fetches a value from a table relative to X ($202F,X).
// Otherwise, it fetches from $2016,X. It then adds $14 and applies a speed modifier.
static void TimerDur_07_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = snes->cpu->x;

    uint8_t val;
    if (ram[0x3558] != 0) {                  // lda $3558 / beq @9f0d
        val = ram[0x202F + x];              // lda $202f,x
    } else {
        val = ram[0x2016 + x];              // lda $2016,x
    }

    // clc / adc #$14
    // In 8-bit mode, this is simple addition. Carry is set if result > 255.
    uint16_t res = (uint16_t)val + 0x14;
    uint8_t final_val = (uint8_t)res;       // Pitfall 7: truncate to 8-bit
    
    snes->cpu->c = (res > 0xFF);            // Update carry flag for consistency

    // tax / stx $a9
    // X is 16-bit (xf=0), so we zero-extend the 8-bit A into X
    uint16_t x_val = (uint16_t)final_val;
    write16(ram, 0xA9, x_val);

    // jsr ApplySpeedMod
    apply_speed_mod_emu(snes);

    // jmp SetTimerDur (Tail call)
    set_timer_dur_emu(snes);
}

// PITFALLS: 7 (Arithmetic truncation to 8-bit for the ADC result)
// HELPERS: apply_speed_mod_emu(snes), set_timer_dur_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=16-bit, y=none
//   inputs_ram:  0x3558=1, 0x202F=1 (relative to x), 0x2016=1 (relative to x)
//   output_ram:  none (ends in jump to SetTimerDur)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (routine ends in JMP, not RTS)

REVERSED_FUNCTION: battle::TimerDur_07 ($9F:03)