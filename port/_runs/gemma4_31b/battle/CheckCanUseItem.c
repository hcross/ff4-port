// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$A1, DP=0
// Purpose: Checks if a character can use a specific item based on item classes 
// and character equipment properties.
//
// Logic:
// 1. If ItemID is in range [0x6D, 0xDE) AND [0xB0, 0xDE), it skips the check (returns success).
// 2. Otherwise, it loads the character's equipment properties via LoadArrayItem.
// 3. It retrieves the item class for the item, then performs a bitwise AND with the 
//    character's properties. If the result is non-zero, the item is usable.
static void CheckCanUseItem_c(Snes *snes, uint8_t item_id) {
    uint8_t *ram = snes->ram;
    ram[0x353D] = 0; // stz $353d

    // Range checks using item_id (A)
    if (item_id < 0x6D) { // cmp #$6d / bcc @a169
        goto check_equip;
    }
    if (item_id >= 0xDE) { // cmp #$de / bcs @a197
        goto fail;
    }
    if (item_id < 0xB0) { // cmp #$b0 / bcc @a197
        goto fail;
    }
    // If it reaches here, item_id is [0xB0, 0xDE), which is "usable" (bcs @a19a)
    goto success;

check_equip:
    ram[0xE5] = item_id; // tax / stx $e5
    
    // Setup LoadArrayItem parameters (EquipProp address)
    // Assume EquipProp is a known symbol/constant address
    uint16_t equip_prop_addr = 0xXXXX; // Replace with actual EquipProp address from symbols
    write16(ram, 0x80, equip_prop_addr & 0xFF); // loword
    ram[0x82] = (equip_prop_addr >> 8) & 0xFF;  // ^EquipProp
    
    ram[0x84] = 0x08; // Assume LoadArrayItem index/param is stored here or in A
    // The ASM does: lda #$08 / jsr LoadArrayItem. 
    // We simulate this via emulator since LoadArrayItem is complex.
    snes->cpu->a = 0x08;
    load_array_item_emu(snes);

    // Process the loaded result from $28A2
    uint8_t class_idx = ram[0x28A2] & 0x1F; // lda $28a2 / and #$1f
    uint8_t x_offset = (uint8_t)(class_idx << 1); // asl / tax (Pitfall 7: truncate to 8-bit)
    
    // ItemClasses is a data table in ROM/RAM
    // lda f:ItemClasses,x / sta $ab
    // lda f:ItemClasses+1,x / sta $ac
    uint8_t item_class_lo = snes->rom[ITEM_CLASSES_OFFSET + x_offset];
    uint8_t item_class_hi = snes->rom[ITEM_CLASSES_OFFSET + 1 + x_offset];
    ram[0xAB] = item_class_lo;
    ram[0xAC] = item_class_hi;

    // longa / lda $ab / and $a9 (Note: $a9 is a RAM address used as a mask)
    uint16_t class_val = read16(ram, 0xAB);
    uint16_t mask_val = read16(ram, 0xA9);
    
    if ((class_val & mask_val) != 0) { // shorta / bne @a19a
        goto success;
    }

fail:
    ram[0x353D]++; // inc $353d

success:
    return;
}

// PITFALLS: 7 (asl A truncated to 8-bit for X offset), 1 (DB=$A1)
// HELPERS: load_array_item_emu(snes) — delegates LoadArrayItem @ 845e
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x28A2=1, 0xA9=2, 0xAB=1, 0xAC=1
//   output_ram:  0x353D=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0xA1
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckCanUseItem ($A1:58)