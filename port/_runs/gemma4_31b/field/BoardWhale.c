#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (Default for board/field), DP=0
// Logic: Sets constants for the "Whale" board state. 
// If ram[$E1] is non-zero, it configures palette zoom and airship animation speed.
// Otherwise, it plays the map song and clears ram[$79].
static void BoardWhale_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1704] = 0x06;
    ram[0xAC] = 3;

    if (ram[0xE1] == 0) { // lda $e1 / beq @a31f
        play_map_song_emu(snes);
        ram[0x79] = 0;     // stz $79
        return;
    }

    ram[0xAD] = 0x30;
    ram[0xB9] = 0x20;

    // lsr A (A was 0x20) -> 0x10
    // Pitfall 7: Truncate to 8-bit result
    uint8_t a = (uint8_t)(0x20 >> 1);
    snes->cpu->a = a; 

    update_zoom_pal_emu(snes);
    ram[0x06FD] = 0x0F; // set airship animation speed
}

// PITFALLS: 7 (LSR A handled as 8-bit truncation)
// HELPERS: update_zoom_pal_emu(snes), play_map_song_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00E1=1
//   output_ram:  0x1704=1, 0x00AC=1, 0x00AD=1, 0x00B9=1, 0x06FD=1, 0x0079=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::BoardWhale ($A2:FF)