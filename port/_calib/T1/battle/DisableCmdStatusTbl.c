#include "snes/snes.h"

// Entry mode: This is not a functional routine, but a data table.
// The ASM source consists entirely of .byte directives.
// In the context of the battle module, this table is accessed via 
// absolute addressing or indexed offsets to determine which status 
// effects disable specific commands.
//
// Logic: 
// Returns a 16-bit bitmask of disabling status effects for a 
// given command ID. Each command occupies 2 bytes (16-bit word).
static uint16_t DisableCmdStatusTbl_c(Snes *snes, uint8_t cmd_id) {
    // The table is located at $FD:19 in the ROM.
    // We map the table to a local array for clarity.
    static const uint16_t table[] = {
        0x0000, // 00: fight
        0x0000, // 01: item
        0x002C, // 02: white magic
        0x0004, // 03: black magic
        0x002C, // 04: summon
        0x0038, // 05: dark wave
        0x0030, // 06: jump 1
        0x002C, // 07: recall
        0x0004, // 08: sing
        0x0000, // 09: hide
        0x0000, // 0a: salve
        0x0000, // 0b: pray
        0x0038, // 0c: aim
        0x0039, // 0d: focus 1
        0x4038, // 0e: kick
        0x0038, // 0f: brace
        0x003C, // 10: twin 1
        0x0038, // 11: bluff
        0x0038, // 12: cry
        0x0030, // 13: cover
        0x0038, // 14: search (peep)
        0x0038, // 15: airship ???
        0x0030, // 16: throw (dart)
        0x0020, // 17: steal (sneak)
        0x002C, // 18: ninjutsu
        0x0038, // 19: regen
        0x0000, // 1a: change row
        0x0000, // 1b: defend
        0x0000, // 1c: appear (show)
        0x0000  // 1d: don't cover (off)
    };

    if (cmd_id >= (sizeof(table) / sizeof(uint16_t))) {
        return 0;
    }
    return table[cmd_id];
}

// PITFALLS: None. This is a static data lookup table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (This is a data table, not a routine)

// REVERSED_FUNCTION: battle::DisableCmdStatusTbl ($FD:19)