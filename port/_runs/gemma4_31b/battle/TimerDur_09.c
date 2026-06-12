// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9F, DP=0
// This routine calculates a timer duration based on whether a specific 
// flag is set at $3558. It computes a base value, scales it via Mult8, 
// adds a constant offset (0x1E), and then passes it through ApplySpeedMod.
static void TimerDur_09_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = snes->cpu->x;

    // 8-bit A mode inherited from battle module convention
    uint8_t val_3558 = ram[0x3558];
    if (val_3558 != 0) { // beq @9f4a
        ram[0xAD] = ram[0x202F + (x & 0xFF)]; // Simplified indexing assuming X is offset
        ram[0xAE] = 0x04;
    } else {
        // clc / lda / adc sequence
        uint8_t a = ram[0x2017 + (x & 0xFF)];
        uint8_t res = (uint8_t)(a + ram[0x2018 + (x & 0xFF)]); // Pitfall 7: truncate to 8-bit
        ram[0xAD] = res;
        ram[0xAE] = 0x02;
    }

    // Set up inputs for Mult8
    ram[0xDF] = ram[0xAD];
    ram[0xE2] = ram[0xAE];

    mult8_emu(snes); // jsr Mult8

    // Handle 16-bit result from Mult8 (stored in $E3-$E4) and add 0x1E
    uint8_t carry = 0;
    uint8_t lo = ram[0xE3];
    uint8_t res_lo = (uint8_t)(lo + 0x1E); // adc #$1e
    carry = (lo > (uint8_t)(0xFF - 0x1E)); // Track carry for the high byte

    ram[0xA9] = res_lo;

    uint8_t hi = ram[0xE4];
    uint8_t res_hi = (uint8_t)(hi + carry); // adc #$00
    ram[0xAA] = res_hi;

    apply_speed_mod_emu(snes); // jsr ApplySpeedMod
    set_timer_dur_emu(snes);    // jmp SetTimerDur
}

// PITFALLS: 7 (Truncation of 8-bit ADC to match 65816 hardware behavior)
// HELPERS: mult8_emu(snes), apply_speed_mod_emu(snes), set_timer_dur_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x3558=1, 0x202F=1, 0x2017=1, 0x2018=1
//   output_ram:  none (results passed to SetTimerDur via 0xA9/0xAA)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9F
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Routine ends in a jump to SetTimerDur, breaking return flow)

REVERSED_FUNCTION: battle::TimerDur_09 ($9F:3A)