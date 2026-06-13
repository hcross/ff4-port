#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x00, DP=0
// Purpose: Sets the player vehicle to Chocobo and updates movement speed,
// then triggers the map music change.
static void BoardChoco_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1704] = 0x01; // lda #$01 / sta $1704 (set vehicle)
    ram[0xAC] = 0x01;   // sta $ac (set movement speed)

    play_map_song_emu(snes); // jsr PlayMapSong (delegated)
}

// PITFALLS: None applicable for this simple linear routine.
// HELPERS: play_map_song_emu(snes) — delegates PlayMapSong @$8D5D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1704=1, 0xAC=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::BoardChoco ($A0:3E)