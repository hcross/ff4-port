// This is a data table, not a routine. It maps command IDs ($00-$1D)
// to bitmasks of status effects that disable the command.
// Each entry is a 16-bit little-endian value.
//
// The table is indexed by command ID * 2 (since each entry is 2 bytes).
// No execution logic — just a static data array.
//
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// No register inputs. No execution flow. Pure data.
//
// Bitmask legend (from context):
//   bit 0: toad
//   bit 1: mini
//   bit 2: pig
//   bit 3: mute
//   bit 4: poison
//   bit 5: float
//   ...
static const uint8_t DisableCmdStatusTbl[0x3C] = {
    0x00, 0x00, // $00: fight
    0x00, 0x00, // $01: item
    0x2c, 0x00, // $02: white magic          toad, pig, mute
    0x04, 0x00, // $03: black magic          mute
    0x2c, 0x00, // $04: summon               toad, pig, mute
    0x38, 0x00, // $05: dark wave            toad, mini, pig
    0x30, 0x00, // $06: jump 1               toad, mini
    0x2c, 0x00, // $07: recall               toad, pig, mute
    0x04, 0x00, // $08: sing                 mute
    0x00, 0x00, // $09: hide
    0x00, 0x00, // $0a: salve
    0x00, 0x00, // $0b: pray
    0x38, 0x00, // $0c: aim                  toad, mini, pig
    0x39, 0x00, // $0d: focus 1              toad, mini, poison
    0x38, 0x40, // $0e: kick                 toad, mini, pig, float
    0x38, 0x00, // $0f: brace                toad, mini, pig
    0x3c, 0x00, // $10: twin 1               toad, mini, pig, mute
    0x38, 0x00, // $11: bluff                toad, mini, pig
    0x38, 0x00, // $12: cry                  toad, mini, pig
    0x30, 0x00, // $13: cover                toad, mini
    0x38, 0x00, // $14: search (peep)        toad, mini, pig
    0x38, 0x00, // $15: airship ???          toad, mini, pig
    0x30, 0x00, // $16: throw (dart)         toad, mini
    0x20, 0x00, // $17: steal (sneak)        toad
    0x2c, 0x00, // $18: ninjutsu             toad, pig, mute
    0x38, 0x00, // $19: regen                toad, mini, pig
    0x00, 0x00, // $1a: change row
    0x00, 0x00, // $1b: defend
    0x00, 0x00, // $1c: appear (show)
    0x00, 0x00, // $1d: don't cover (off)
};

// PITFALLS: None (data table, no execution)
// HELPERS: None (no calls)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
REVERSED_FUNCTION: battle::DisableCmdStatusTbl ($FD:0019)