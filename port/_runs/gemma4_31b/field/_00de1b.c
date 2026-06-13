#include "snes/snes.h"

// Logic:
// 1. Set player control enable flag ($D5 = 1)
// 2. Call world movement validation and execution routines
// 3. Set input/state flag ($AD = 0x20)
static void _00de1b_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xD5] = 0x01;                 // lda #$01 / sta $d5

    check_player_move_world_emu(snes); // jsr CheckPlayerMoveWorld
    move_player_emu(snes);            // jsr MovePlayer

    ram[0xAD] = 0x20;                 // lda #$20 / sta $ad
}

// PITFALLS: None. Simple linear flow with absolute 8-bit writes.
// HELPERS: 
//   check_player_move_world_emu(snes) - delegates CheckPlayerMoveWorld @ $AB84
//   move_player_emu(snes)             - delegates MovePlayer @ $A871
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xD5=1, 0xAD=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00de1b ($DE:1B)