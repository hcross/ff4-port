// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine calculates a timer duration based on a base value in $397B,
// applies a 2x multiplier, adds a constant offset (30), and then
// passes the result through speed modification and final timer setting.
//
// Logic:
//   val = ram[0x397B]
//   temp16 = (val << 1) + 30
//   ram[0xA9-AA] = temp16
//   ApplySpeedMod()
//   SetTimerDur()
static void TimerDur_08_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $397b / sta $ad / stz $ae
    uint8_t val = ram[0x397B];
    ram[0xAD] = val;
    ram[0xAE] = 0;

    // asl $ad / rol $ae (16-bit shift left)
    uint16_t temp = (uint16_t)val << 1; 
    ram[0xAD] = (uint8_t)(temp & 0xFF);
    ram[0xAE] = (uint8_t)(temp >> 8);

    // clc / lda $ad / adc #$1e / sta $a9
    // lda $ae / adc #$00 / sta $aa
    // Result is (val * 2) + 30 (0x1E)
    uint16_t result = (uint16_t)temp + 30;
    ram[0xA9] = (uint8_t)(result & 0xFF);
    ram[0xAA] = (uint8_t)(result >> 8);

    apply_speed_mod_emu(snes); // jsr ApplySpeedMod
    set_timer_dur_emu(snes);    // jmp SetTimerDur
}

// PITFALLS: 7 (Arithmetic truncation: used uint16_t to handle 8-bit 
// addition overflow into the high byte accurately).
// HELPERS: apply_speed_mod_emu(snes) — delegates ApplySpeedMod @ $9F:D8
//          set_timer_dur_emu(snes) — delegates SetTimerDur @ $9F:CF
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x397B=1
//   output_ram:  none (effects are passed to SetTimerDur)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::TimerDur_08 ($9F:1C)