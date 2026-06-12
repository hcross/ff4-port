// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = item_id (8-bit)
// Logic:
//   if item_id >= 0x6D:
//     if item_id >= 0xDE || item_id < 0xB0:
//       set $353d = 1 (cannot use)
//     else:
//       check if character can equip item via EquipProp table
//       if not, set $353d = 1
//   else:
//     check if character can equip item via EquipProp table
//     if not, set $353d = 1
static void CheckCanUseItem_c(Snes *snes, uint8_t item_id) {
    uint8_t *ram = snes->ram;
    ram[0x353D] = 0;

    if (item_id >= 0x6D) {
        if (item_id >= 0xDE || item_id < 0xB0) {
            ram[0x353D] = 1;
            return;
        }
    }

    // Load EquipProp entry for item_id
    ram[0xE5] = item_id;
    write16(ram, 0x80, 0x5842); // EquipProp table loword
    ram[0x82] = 0x03;           // EquipProp table bank
    load_array_item_emu(snes, 0x08);

    // Get item equipability class bits
    uint16_t equip_class = read16(ram, 0x28A2) & 0x1F;
    uint16_t index = equip_class << 1;
    ram[0xAB] = ram[0x5842 + index];
    ram[0xAC] = ram[0x5843 + index];

    // Check if character can use this item type
    uint16_t ab = read16(ram, 0xAB);
    uint16_t a9 = read16(ram, 0xA9);
    if ((ab & a9) != 0) {
        return; // character can use item
    }

    ram[0x353D] = 1;
}

// PITFALLS: 1 (DB must be $7E for absolute addressing to hit WRAM),
//           6 (mode A starts as 8-bit, longa/shorta used internally),
//           7 (16-bit operations must truncate properly when switching back to 8-bit)
// HELPERS: load_array_item_emu(snes, 0x08) — delegates LoadArrayItem @ $03:845E
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0xA9=2
//   output_ram:  0x353D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckCanUseItem ($A1:58)