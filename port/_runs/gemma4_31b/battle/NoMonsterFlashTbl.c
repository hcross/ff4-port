// This is a data table rather than an executable routine.
// It contains a sequence of flash-duration or pattern values 
// used to determine if a monster should skip the flash effect.
static uint8_t NoMonsterFlashTbl_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x16,
    0x51, 0x0D, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00
};

// Since this is a table in ROM/WRAM, the "translation" in the context of a 
// C-native reimplementation is the definition of the data array.
// If the parity harness expects this to be accessed via snes->ram, 
// the harness must ensure this table is loaded at the mapped address.
static uint8_t get_no_monster_flash_val(Snes *snes, uint8_t index) {
    // The table is located at $FE:D4
    // Relative to the WRAM mapping if mirrored or treated as an offset
    if (index >= sizeof(NoMonsterFlashTbl_data)) return 0;
    return NoMonsterFlashTbl_data[index];
}

// PITFALLS: None. This is static data.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: auto
//   CUSTOM_SPIKE: yes (Data table, not a routine)

REVERSED_FUNCTION: battle::NoMonsterFlashTbl ($FE:D4)