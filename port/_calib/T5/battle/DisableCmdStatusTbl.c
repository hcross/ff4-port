#include "snes/snes.h"

// DisableCmdStatusTbl is a ROM data table (not executable code).
// It maps each battle command index (0x00–0x1D) to a 2-byte status
// bitmask that disables the command when the actor has those statuses.
//
// The C translation returns a pointer to a static const array that
// matches the original ROM bytes at $FD:19–$FD:54.
static const uint8_t *DisableCmdStatusTbl_c(void) {
    static const uint8_t table[60] = {
        0x00, 0x00, // 0x00: fight
        0x00, 0x00, // 0x01: item
        0x2C, 0x00, // 0x02: white magic          (toad, pig, mute)
        0x04, 0x00, // 0x03: black magic          (mute)
        0x2C, 0x00, // 0x04: summon               (toad, pig, mute)
        0x38, 0x00, // 0x05: dark wave            (toad, mini, pig)
        0x30, 0x00, // 0x06: jump 1               (toad, mini)
        0x2C, 0x00, // 0x07: recall               (toad, pig, mute)
        0x04, 0x00, // 0x08: sing                 (mute)
        0x00, 0x00, // 0x09: hide
        0x00, 0x00, // 0x0A: salve
        0x00, 0x00, // 0x0B: pray
        0x38, 0x00, // 0x0C: aim                  (toad, mini, pig)
        0x39, 0x00, // 0x0D: focus 1              (toad, mini, poison)
        0x38, 0x40, // 0x0E: kick                 (toad, mini, pig, float)
        0x38, 0x00, // 0x0F: brace                (toad, mini, pig)
        0x3C, 0x00, // 0x10: twin 1               (toad, mini, pig, mute)
        0x38, 0x00, // 0x11: bluff                (toad, mini, pig)
        0x38, 0x00, // 0x12: cry                  (toad, mini, pig)
        0x30, 0x00, // 0x13: cover                (toad, mini)
        0x38, 0x00, // 0x14: search (peep)        (toad, mini, pig)
        0x38, 0x00, // 0x15: airship ???          (toad, mini, pig)
        0x30, 0x00, // 0x16: throw (dart)         (toad, mini)
        0x20, 0x00, // 0x17: steal (sneak)        (toad)
        0x2C, 0x00, // 0x18: ninjutsu             (toad, pig, mute)
        0x38, 0x00, // 0x19: regen                (toad, mini, pig)
        0x00, 0x00, // 0x1A: change row
        0x00, 0x00, // 0x1B: defend
        0x00, 0x00, // 0x1C: appear (show)
        0x00, 0x00, // 0x1D: don't cover (off)
    };
    return table;
}

// PITFALLS: none (data table, no executable code)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  n/a
//   entry_flags: n/a
//   CUSTOM_SPIKE: yes  (data table; auto-spike generator cannot handle)
// REVERSED_FUNCTION: battle::DisableCmdStatusTbl ($FD:19)