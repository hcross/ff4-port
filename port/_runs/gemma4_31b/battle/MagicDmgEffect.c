// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: 
//   Checks for elemental strengths/weaknesses.
//   If a strong element is found (indicated by $38FE bit 7), it clears that bit and jumps to a specific handler.
//   Otherwise, checks for weaknesses and specific monster/state flags ($2704, $352A, $26D2) 
//   to determine if damage calculation should be skipped.
static void MagicDmgEffect_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    check_strong_elem_emu(snes); // jsr CheckStrongElem

    uint8_t strong_flag = ram[0x38FE];
    if (strong_flag < 0x80) { // bpl @d388 (Bit 7 is 0)
        goto check_weak;
    }

    // Strong element found (Bit 7 is 1)
    ram[0x38FE] = strong_flag & 0x7F; // and #$7f / sta $38fe
    _d416_emu(snes); // jmp _d416
    return;

check_weak:
    check_weak_elem_emu(snes); // jsr CheckWeakElem

    if ((ram[0x2704] & 0x40) == 0) { // and #$40 / beq _d3ae
        goto calc_dmg;
    }

    if (ram[0x352A] != 0) { // lda $352a / bne @d3a7
        uint8_t id = ram[0x26D2];
        if (id == 0xC7) { // cmp #$c7 / beq _d3b1
            return; // beq _d3b1
        }
    } else {
        uint8_t id = ram[0x26D2];
        if (id == 0x28 || id == 0x55) { // cmp #$28 / beq _d3b1 / cmp #$55 / beq _d3b1
            return; // beq _d3b1
        }
        if (id == 0xA1) { // cmp #$a1 / bne _d3ae
            return; // bne _d3ae (branch NOT taken -> return)
        }
    }

calc_dmg:
    calc_dmg_emu(snes); // jsr CalcDmg
}

// PITFALLS: 1 (DB=$7E assumed), 6 (A 8-bit mode used for logic/masks)
// HELPERS: check_strong_elem_emu, check_weak_elem_emu, calc_dmg_emu, _d416_emu
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x38FE=1, 0x2704=1, 0x352A=1, 0x26D2=1
//   output_ram:  0x38FE=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicDmgEffect ($D3:78)