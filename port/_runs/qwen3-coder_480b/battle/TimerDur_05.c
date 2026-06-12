// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: X = entity index (0-3 for character, 4-7 for monster)
// Logic:
//   if ($3558 != 0)  // monster
//     value = $202F,x  // level + 10
//   else
//     value = $2018,x  // modified spirit
//   $AD = value
//   timer = 300 - 4 * value
//   timer = max(timer, 1)
//   timer = ApplySpeedMod(timer)
//   timer = timer / 6
//   SetTimerDur(timer)
static void TimerDur_05_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = snes->cpu->x;  // X is 16-bit (xf=0)

    uint8_t value;
    if (ram[0x3558] == 0) {         // beq @9ecd
        value = ram[0x2018 + x];    // lda $2018,x
    } else {
        value = ram[0x202F + x];    // lda $202f,x
    }

    ram[0xAD] = value;              // sta $ad

    uint8_t ad4 = (uint8_t)(value << 2);  // asl $ad / asl $ad (8-bit, truncates)
    ram[0xAD] = ad4;

    // Compute 300 - 4*value with carry
    snes->cpu->c = true;            // sec
    uint16_t timer = 0x2C - ad4;    // lda #$2c / sbc $ad
    if (!snes->cpu->c) {            // bcs @9eea (carry clear = underflow)
        timer = 1;                  // ldx #1 / stx $a9
    }

    write16(ram, 0xA9, timer);      // sta $a9 / lda #$01 / sbc #$00 / sta $aa

    ApplySpeedMod_emu(snes);        // jsr ApplySpeedMod

    write16(ram, 0x3945, read16(ram, 0xAB));  // ldx $ab / stx $3945
    write16(ram, 0x3947, 6);                  // ldx #6 / stx $3947

    Div16_emu(snes);                          // jsr Div16

    write16(ram, 0xAB, read16(ram, 0x3949));  // ldx $3949 / stx $ab

    SetTimerDur_emu(snes);                    // jmp SetTimerDur
}

// PITFALLS: 1 (DB=$7E required for WRAM access), 3 (bcs sense),
//           7 (ASL truncation in 8-bit mode)
// HELPERS: ApplySpeedMod_emu(snes), Div16_emu(snes), SetTimerDur_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x3558=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::TimerDur_05 ($9E:C3)