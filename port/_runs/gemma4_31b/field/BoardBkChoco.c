#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$A0, DP=0
// Logic:
//   Increments a counter ($1715). If the counter is exactly 1, 
//   copies values from $1706-7 to $1716-7.
//   Then sets the map song ID to 2 and triggers music playback.
static void BoardBkChoco_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1715]++;                               // inc $1715
    uint8_t counter = ram[0x1715];               // lda $1715
    
    if (counter == 0x01) {                       // cmp #$01 / bne @a05f (inverted)
        ram[0x1716] = ram[0x1706];               // lda $1706 / sta $1716
        ram[0x1717] = ram[0x1707];               // lda $1707 / sta $1717
    }

    ram[0x1704] = 0x02;                          // lda #$02 / sta $1704
    ram[0xAC] = 0x02;                            // sta $ac
    play_map_song_emu(snes);                    // jsr PlayMapSong
}

// PITFALLS: 7 (8-bit increment truncation handled by uint8_t)
// HELPERS: play_map_song_emu(snes) — delegates PlayMapSong @ $8D5D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1715=1, 0x1706=1, 0x1707=1
//   output_ram:  0x1716=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xA0
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::BoardBkChoco ($A0:49)