#include "snes/snes.h"

// Logic:
//   1. Loop 16 times: animate, decrement $B5 (timer/counter), increment $79.
//   2. Check if the current tile (bit 3 of $A1) allows landing (forest tile).
//   3. If no: jump to LiftoffBkChoco (re-launch).
//   4. If yes: reset landing flags ($1704, $AC, $7B), set state to $02 ($1705).
//   5. Copy $1706 to $1713.
//   6. If $1715 == 2: clear $1715 and $1712.
//   7. Play map song and enable tent/save ($1A02).
static void LandBkChoco_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x79] = 0; // stz $79
    for (uint8_t i = 0; i < 0x10; i++) { // loop @a122
        animate4_emu(snes);              // jsr Animate4
        ram[0xB5]--;                     // dec $b5
        ram[0x79]++;                     // inc $79
        // lda $79 / cmp #$10 / bne @a122
    }

    if (!(ram[0xA1] & 0x08)) {            // lda $a1 / and #$08 / bne @a138
        liftoff_bk_choco_emu(snes);       // jmp LiftoffBkChoco
        return;
    }

    ram[0x1704] = 0;                     // stz $1704
    ram[0xAC] = 0;                       // stz $ac
    ram[0x7B] = 0;                       // stz $7b
    ram[0x1705] = 0x02;                  // lda #$02 / sta $1705
    ram[0x1713] = ram[0x1706];           // ldx $1706 / stx $1713

    if (ram[0x1715] == 0x02) {           // lda $1715 / cmp #$02 / bne @a157
        ram[0x1715] = 0;                 // stz $1715
        ram[0x1712] = 0;                 // stz $1712
    }

    play_map_song_emu(snes);             // jsr PlayMapSong
    ram[0x1A02]++;                       // inc $1a02
}

// PITFALLS: 1 (DB=$7E is standard for field/battle WRAM access in these routines)
// HELPERS: animate4_emu(snes), liftoff_bk_choco_emu(snes), play_map_song_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x79=1, 0xB5=1, 0xA1=1, 0x1706=1, 0x1715=1
//   output_ram:  0x1A02=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LandBkChoco ($A1:20)