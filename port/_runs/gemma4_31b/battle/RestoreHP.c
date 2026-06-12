// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Calculate HP restoration. Checks for specific game state flags 
// to determine if a fixed restoration value should be used or if the 
// general CalcDmg routine should be called.
static void RestoreHP_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // @d40c: lda $2740 / and #$80 / beq _d416
    if (ram[0x2740] & 0x80) {
        // jmp _d3ae (External jump to a different routine/segment)
        // Note: The provided asm shows a jump to _d3ae. In the parity 
        // harness, this is handled by delegating the jump target.
        jump_d3ae_emu(snes); 
        return;
    }

    // _d416: logic to check for specific fixed-restore conditions
    bool use_fixed_restore = false;
    if (ram[0x352A] == 0) {                      // bne @d439
        if (ram[0x26D2] == 0x11 && ram[0x3906] == 0x01) { // cmp #$11 / cmp #$01
            use_fixed_restore = true;
        }
    }

    if (use_fixed_restore) {
        // longa / sec / lda $2709 / sbc $2707 / sta $a4 / shorta0
        // Calculate 16-bit difference: $2709 - $2707
        uint16_t val = read16(ram, 0x2709);
        uint16_t sub = read16(ram, 0x2707);
        uint16_t res = (uint16_t)(val - sub);
        write16(ram, 0xA4, res);
        // shorta0 effectively resets A to DP (0) and sets mode to 8-bit
    } else {
        // @d439: jsr CalcDmg
        calc_dmg_emu(snes);
    }

    // @d43c: lda $a5 / ora #$80 / sta $a5
    ram[0xA5] |= 0x80;
}

// PITFALLS: 1 (DB=$7E), 6 (Mode A toggled via longa/shorta0 for 16-bit subtraction)
// HELPERS: calc_dmg_emu(snes), jump_d3ae_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2740=1, 0x352A=1, 0x26D2=1, 0x3906=1, 0x2707=2, 0x2709=2
//   output_ram:  0xA5=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (due to jump to _d3ae)
REVERSED_FUNCTION: battle::RestoreHP ($D4:0C)