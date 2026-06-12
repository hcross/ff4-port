// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9E, DP=0
// Logic:
//   1. Determine "mod spirit" value based on if target is a character ($3558) 
//      or monster.
//   2. Calculate: duration = 300 - (4 * mod_spirit).
//   3. Ensure duration is at least 1.
//   4. Apply speed modification (ApplySpeedMod).
//   5. Divide resulting duration by 6 (Div16).
//   6. Jump to SetTimerDur to finalize.
static void TimerDur_05_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    uint8_t mod_spirit;
    if (ram[0x3558] != 0) {      // lda $3558 / beq @9ecd
        mod_spirit = ram[0x202F + cpu->x]; // lda $202f,x
    } else {
        mod_spirit = ram[0x2018 + cpu->x]; // lda $2018,x
    }

    ram[0xAD] = mod_spirit;    // sta $ad
    uint8_t shifted = mod_spirit;
    shifted = (uint8_t)(shifted << 1); // asl $ad (Pitfall 7)
    shifted = (uint8_t)(shifted << 1); // asl $ad (Pitfall 7)
    
    // Calculate 300 - (4 * mod_spirit)
    // 300 = 0x012C. Assembly uses 8-bit SBC with carry.
    uint8_t low = 0x2C;
    uint8_t high = 0x01;
    uint8_t sub_val = shifted;

    // sbc $ad (low byte)
    uint8_t res_low = (uint8_t)(low - sub_val); 
    bool carry = (low < sub_val); 

    // sbc #$00 (high byte)
    // sbc high_byte - 0 - carry
    uint8_t res_high = (uint8_t)(high - 0 - carry);
    bool final_carry = (high < (carry + 0));

    ram[0xA9] = res_low;
    ram[0xAA] = res_high;

    if (!final_carry) {          // bcs @9eea (branches if A >= 0)
        ram[0xA9] = 1;           // ldx #1 / stx $a9
    }

    apply_speed_mod_emu(snes);   // jsr ApplySpeedMod

    // Divide result by 6
    write16(ram, 0x3945, read16(ram, 0xAB)); // ldx $ab / stx $3945
    write16(ram, 0x3947, 6);                  // ldx #6 / stx $3947
    div16_emu(snes);                         // jsr Div16
    write16(ram, 0xAB, read16(ram, 0x3949)); // ldx $3949 / stx $ab

    set_timer_dur_emu(snes);     // jmp SetTimerDur
}

// PITFALLS: 7 (Truncation on asl $ad), 1 (DB=0x9E)
// HELPERS: apply_speed_mod_emu, div16_emu, set_timer_dur_emu
// CONTRACT:
//   inputs_reg:  a=none, x=16bit, y=none
//   inputs_ram:  0x3558=1, 0x202F=1, 0x2018=1
//   output_ram:  none (jumps to SetTimerDur)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (ends in jump)

REVERSED_FUNCTION: battle::TimerDur_05 ($9E:C3)