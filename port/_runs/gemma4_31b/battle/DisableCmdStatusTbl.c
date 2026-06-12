/**
 * This routine is not a functional block of code, but a data table
 * mapping command IDs to status effect bitmasks that disable those commands.
 * Each entry is a 16-bit word (little-endian).
 * 
 * Logic: The caller likely uses this table to check if the current
 * character status (bitmask) disables the selected command.
 */
static uint16_t get_disable_cmd_status(Snes *snes, uint8_t cmd_id) {
    // The table is located at $FD:19. 
    // Since it's a static table, we can represent it as a C array.
    static const uint16_t disable_tbl[] = {
        0x0000, // 00: fight
        0x0000, // 01: item
        0x002C, // 02: white magic (toad, pig, mute)
        0x0004, // 03: black magic (mute)
        0x002C, // 04: summon (toad, pig, mute)
        0x0038, // 05: dark wave (toad, mini, pig)
        0x0030, // 06: jump 1 (toad, mini)
        0x002C, // 07: recall (toad, pig, mute)
        0x0004, // 08: sing (mute)
        0x0000, // 09: hide
        0x0000, // 0A: salve
        0x0000, // 0B: pray
        0x0038, // 0C: aim (toad, mini, pig)
        0x0039, // 0D: focus 1 (toad, mini, poison)
        0x4038, // 0E: kick (toad, mini, pig, float)
        0x0038, // 0F: brace (toad, mini, pig)
        0x003C, // 10: twin 1 (toad, mini, pig, mute)
        0x0038, // 11: bluff (toad, mini, pig)
        0x0038, // 12: cry (toad, mini, pig)
        0x0030, // 13: cover (toad, mini)
        0x0038, // 14: search (peep) (toad, mini, pig)
        0x0038, // 15: airship ??? (toad, mini, pig)
        0x0030, // 16: throw (dart) (toad, mini)
        0x0020, // 17: steal (sneak) (toad)
        0x002C, // 18: ninjutsu (toad, pig, mute)
        0x0038, // 19: regen (toad, mini, pig)
        0x0000, // 1A: change row
        0x0000, // 1B: defend
        0x0000, // 1C: appear (show)
        0x0000  // 1D: don't cover (off)
    };

    if (cmd_id >= (sizeof(disable_tbl) / sizeof(uint16_t))) {
        return 0;
    }
    return disable_tbl[cmd_id];
}

// PITFALLS: None. This is a data table translation.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFD
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::DisableCmdStatusTbl ($FD:19)