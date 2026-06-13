#include "snes/snes.h"

// ROM data table: 30 entries (command indices $00-$1D), each 2 bytes LE.
// Byte 0 = status bitfield that disables the command (toad, mini, pig, mute, etc.)
// Byte 1 = additional flags (e.g. bit 6 = float for kick).
// Indexed by command id * 2.
static const uint8_t kDisableCmdStatusTbl[60] = {
    0x00,0x00, // $00: fight
    0x00,0x00, // $01: item
    0x2c,0x00, // $02: white magic
    0x04,0x00, // $03: black magic
    0x2c,0x00, // $04: summon
    0x38,0x00, // $05: dark wave
    0x30,0x00, // $06: jump 1
    0x2c,0x00, // $07: recall
    0x04,0x00, // $08: sing
    0x00,0x00, // $09: hide
    0x00,0x00, // $0a: salve
    0x00,0x00, // $0b: pray
    0x38,0x00, // $0c: aim
    0x39,0x00, // $0d: focus 1
    0x38,0x40, // $0e: kick
    0x38,0x00, // $0f: brace
    0x3c,0x00, // $10: twin 1
    0x38,0x00, // $11: bluff
    0x38,0x00, // $12: cry
    0x30,0x00, // $13: cover
    0x38,0x00, // $14: search (peep)
    0x38,0x00, // $15: airship
    0x30,0x00, // $16: throw (dart)
    0x20,0x00, // $17: steal (sneak)
    0x2c,0x00, // $18: ninjutsu
    0x38,0x00, // $19: regen
    0x00,0x00, // $1a: change row
    0x00,0x00, // $1b: defend
    0x00,0x00, // $1c: appear (show)
    0x00,0x00, // $1d: don't cover (off)
};

// Accessor matching 65816 indexed read: lda DisableCmdStatusTbl,Y (Y even)
static inline uint16_t DisableCmdStatusTbl_read(int cmd_id) {
    int off = cmd_id * 2;
    return (uint16_t)(kDisableCmdStatusTbl[off] | (kDisableCmdStatusTbl[off + 1] << 8));
}