// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: ram[$D0] = char_id
// Output: ram[$D0] = 0xFF if invalid, unchanged otherwise
// Logic:
//   if char_id == 0xFF → invalid
//   else:
//     SelectObj(char_id)
//     x = $a6 (object index)
//     if (obj.flags1 & 0xC0)        → dead/stone → invalid
//     if (obj.flags2 & 0x3C)        → status effects → invalid
//     if (obj.flags3 & 0xC6) == 0   → magnetized/stopped/etc → invalid
//   On any invalid condition, set $d0 = 0xFF
static void ValidateChar_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t char_id = ram[0xD0];

    if (char_id == 0xFF) {           // cmp #$ff / beq @a5ad
        ram[0xD0] = 0xFF;
        return;
    }

    // jsr SelectObj (modifies $a6)
    select_obj_emu(snes);            // delegated call

    uint16_t x = read16(ram, 0xA6);  // ldx $a6 (X is 16-bit)
    uint8_t flags1 = ram[0x2003 + x];
    if ((flags1 & 0xC0) != 0) {      // and #$c0 / bne @a5ad
        ram[0xD0] = 0xFF;
        return;
    }

    uint8_t flags2 = ram[0x2004 + x];
    if ((flags2 & 0x3C) != 0) {      // and #$3c / bne @a5ad
        ram[0xD0] = 0xFF;
        return;
    }

    uint8_t flags3 = ram[0x2005 + x];
    if ((flags3 & 0xC6) == 0) {      // and #$c6 / beq @a5b1 (branch if zero)
        ram[0xD0] = 0xFF;
        return;
    }

    // fallthrough: valid character, $d0 unchanged
}

// PITFALLS: 1 (DB=$7E for WRAM access), 9 (X is 16-bit, used as offset)
// HELPERS: select_obj_emu(snes) — delegates SelectObj @ $00:8489
// CONTRACT:
//   inputs_ram:  0xD0=1
//   output_ram:  0xD0=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::ValidateChar ($A5:8D)