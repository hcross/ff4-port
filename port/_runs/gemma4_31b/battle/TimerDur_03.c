// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine calculates the action timer duration based on the action property.
// 1. Loads action index from $397B, multiplies by 6 using Mult8.
// 2. Uses the result as an index into AttackProp to get a delay value.
// 3. Masks the delay to 5 bits, multiplies by 2.
// 4. If not in auto-battle ($388B == 0), the value is preserved.
// 5. If in auto-battle, the value is zeroed.
// 6. Applies speed modifiers and jumps to SetTimerDur.
static void TimerDur_03_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $397b / sta $df
    ram[0xDF] = ram[0x397B];
    // lda #$06 / sta $e1
    ram[0xE1] = 0x06;

    // jsr Mult8 (delegated)
    // Mult8 typically takes inputs from $DF and $E1, result in $E3
    mult8_emu(snes);

    // ldx $e3 / lda f:AttackProp,x
    uint16_t index = (uint16_t)ram[0xE3];
    // AttackProp is in ROM (bank f). In this C translation, we assume 
    // the emulator helper or a ROM read function handles the mapping.
    // For parity, we simulate the result via the emulated state or a ROM read.
    uint8_t prop = snes->rom[0x0F0000 + index]; // Hypothetical ROM access

    // and #$1f / tax / stx $a9
    uint8_t delay = prop & 0x1F;
    ram[0xA9] = delay;

    // asl $a9 / rol $aa (Multiply by 2)
    // Bit 7 of $A9 goes into Bit 0 of $AA
    uint8_t carry = (ram[0xA9] & 0x80) != 0;
    ram[0xA9] = (uint8_t)(ram[0xA9] << 1); // Pitfall 7: truncate to 8-bit
    ram[0xAA] = (uint8_t)((ram[0xAA] << 1) | carry);

    // lda $388b / beq @9ebd
    if (ram[0x388B] == 0) {
        // Not auto-battle: keep value in $A9/$AA
    } else {
        // Auto-battle: clr_ax / stx $a9
        // clr_ax is tdc / tax. With DP=0, this sets X to 0.
        ram[0xA9] = 0;
        ram[0xAA] = 0;
    }

    // jsr ApplySpeedMod (delegated)
    apply_speed_mod_emu(snes);

    // jmp SetTimerDur (delegated via wrapper since it's a jump to another routine)
    set_timer_dur_emu(snes);
}

// PITFALLS: 7 (asl/rol truncated to 8-bit), 5 (clr_ax acts as zeroing given DP=0)
// HELPERS: mult8_emu(snes), apply_speed_mod_emu(snes), set_timer_dur_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x397B=1, 0x388B=1, 0x0F0000+offset=1
//   output_ram:  none (jumps to SetTimerDur)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Routine ends in a jump to SetTimerDur, not an RTS)

REVERSED_FUNCTION: battle::TimerDur_03 ($9E:99)