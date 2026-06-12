// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$A1, DP=0
// Purpose: Determines if a character can use a specific item.
// It checks if the item falls into ranges that bypass the check, 
// otherwise it loads character equipment properties and compares 
// them against the item's class bitmask.
static void CheckCanUseItem_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    ram[0x353D] = 0; // stz $353d

    uint8_t item_id = cpu->a;

    // Range checks for item_id
    if (item_id < 0x6D) { // cmp #$6d / bcc @a169
        goto check_equip;
    }
    if (item_id >= 0xDE) { // cmp #$de / bcs @a197
        goto fail;
    }
    if (item_id < 0xB0) { // cmp #$b0 / bcc @a197
        goto fail;
    }
    // item_id is [0xB0, 0xDE)
    goto success;

check_equip:
    ram[0xE5] = item_id; // tax / stx $e5

    // Setup LoadArrayItem: EquipProp address (External symbol)
    // Based on disassembly: EquipProp is at $80-$83
    uint16_t equip_prop = 0x3B62; // Actual EquipProp value from symbol table
    write16(ram, 0x80, equip_prop); // ldx #.loword / stx $80 / lda #^ / sta $82
    
    cpu->a = 0x08; // lda #$08
    LoadArrayItem_emu(snes); // jsr LoadArrayItem

    // Process the loaded result from $28A2
    uint8_t class_idx = ram[0x28A2] & 0x1F; // lda $28a2 / and #$1f
    uint8_t x_offset = (uint8_t)(class_idx << 1); // asl / tax (Pitfall 7)

    // ItemClasses is in ROM (f:ItemClasses)
    // The indices are bytes: ItemClasses[x] and ItemClasses[x+1]
    uint8_t item_class_lo = snes->rom[0x018C0 + x_offset]; // f:ItemClasses
    uint8_t item_class_hi = snes->rom[0x018C0 + 1 + x_offset]; // f:ItemClasses+1
    ram[0xAB] = item_class_lo;
    ram[0xAC] = item_class_hi;

    // longa / lda $ab / and $a9
    uint16_t class_mask = read16(ram, 0xAB);
    uint16_t char_props = read16(ram, 0xA9);
    
    if ((class_mask & char_props) != 0) { // shorta / bne @a19a
        goto success;
    }

fail:
    ram[0x353D]++; // inc $353d

success:
    // longa / shorta0 (A = D, then A 8-bit)
    cpu->a = (uint8_t)cpu->dp; // simplified shorta0 behavior for DP=0
    return;
}

// PITFALLS: 1 (DB=$A1), 7 (asl A truncated to 8-bit for X offset)
// HELPERS: LoadArrayItem_emu(snes) — delegates LoadArrayItem @ 845e
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x28A2=1, 0xA9=2
//   output_ram:  0x353D=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0xA1
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckCanUseItem ($A1:58)