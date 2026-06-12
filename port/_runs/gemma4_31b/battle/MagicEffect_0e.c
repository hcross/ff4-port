// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   1. If base magic defense ($2724) is 0xFF, call RemoveTarget.
//   2. If the battle ID is 0x1B7 (ram[0x1800]=0xB7 and ram[0x1801]=0), skip the buff.
//   3. Otherwise, increment base magic defense by 3, capping at 0xFF.
static void MagicEffect_0e_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t mag_def = ram[0x2724];
    if (mag_def == 0xFF) {       // cmp #$ff / bne @d86d
        remove_target_emu(snes);  // jmp RemoveTarget
        return;
    }

    // Check if battle is $01B7 (ZeroMus)
    if (ram[0x1800] == 0xB7 && ram[0x1801] == 0) { // cmp #$b7 / bne @d879 ... bne @d886
        return;
    }

    // Increment magic defense by 3, cap at 0xFF
    uint8_t result = (uint8_t)(mag_def + 3); // adc #$03 (Pitfall 7)
    if (result < mag_def) {                  // bcc @d883 (Carry set if overflow)
        result = 0xFF;
    }
    ram[0x2724] = result;                    // sta $2724
}

// PITFALLS: 7 (Arithmetic truncation: (mag_def + 3) must be cast to uint8_t
// to match 65816 carry behavior for the 0xFF cap check)
// HELPERS: remove_target_emu(snes) — delegates RemoveTarget @ $E030
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2724=1, 0x1800=1, 0x1801=1
//   output_ram:  0x2724=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_0e ($D8:63)