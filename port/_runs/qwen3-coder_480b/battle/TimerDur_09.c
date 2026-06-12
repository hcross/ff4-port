// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: X = entity index (used to index into $2000+ structures)
// Logic:
//   if ($3558 != 0) {
//     $ad = $202f[x]; $ae = 4
//   } else {
//     $ad = $2017[x] + $2018[x]; $ae = 2
//   }
//   $df = $ad; $e2 = $ae
//   Mult8() → result in $e3-$e4
//   $a9-$aa = $e3-$e4 + 0x1e
//   ApplySpeedMod()
//   SetTimerDur()
static void TimerDur_09_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = snes->cpu->x;  // X is 16-bit (xf=0)

    uint8_t cond = ram[0x3558];
    if (cond != 0) {                    // beq @9f4a
        ram[0xAD] = ram[0x202F + x];    // lda $202f,x / sta $ad
        ram[0xAE] = 4;                  // lda #$04 / sta $ae
    } else {
        uint8_t base = ram[0x2017 + x];         // lda $2017,x
        uint8_t add = ram[0x2018 + x];          // lda $2018,x
        ram[0xAD] = (uint8_t)(base + add);      // clc / adc / sta $ad (8-bit truncation)
        ram[0xAE] = 2;                          // lda #$02 / sta $ae
    }

    ram[0xDF] = ram[0xAD];              // lda $ad / sta $df
    ram[0xE2] = ram[0xAE];              // lda $ae / sta $e2

    mult8_emu(snes);                    // jsr Mult8

    uint8_t e3 = ram[0xE3];
    uint8_t e4 = ram[0xE4];
    ram[0xA9] = (uint8_t)(e3 + 0x1E);           // clc / lda $e3 / adc #$1e / sta $a9
    ram[0xAA] = (uint8_t)(e4 + (ram[0xA9] < (uint8_t)(e3 + 0x1E) ? 1 : 0)); // adc #$00

    apply_speed_mod_emu(snes);          // jsr ApplySpeedMod
    set_timer_dur_emu(snes);            // jmp SetTimerDur (tail call)
}

// PITFALLS: 1 (DB=$7E), 6 (mode A 8-bit), 7 (arithmetic truncation)
// HELPERS: mult8_emu, apply_speed_mod_emu, set_timer_dur_emu
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x3558=1, 0x2017=1, 0x2018=1, 0x202f=1
//   output_ram:  none  (SetTimerDur writes final output)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::TimerDur_09 ($9F:3A)