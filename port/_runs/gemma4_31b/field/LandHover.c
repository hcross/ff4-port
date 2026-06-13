#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine handles the "hover" landing animation for a character/object.
// If the hover flag is set, it plays an animation loop 4 times, 
// updates state registers, and triggers the map song.
static void LandHover_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $a1 / and #$01 / bne @a1e5
    if ((ram[0xA1] & 0x01) == 0) {
        return;
    }

    // @a1e5: stz $79
    ram[0x79] = 0;

    // @a1e7 loop: 4 iterations
    do {
        animate8_emu(snes);      // jsr Animate8
        ram[0xB6]--;              // dec $b6
        ram[0x79]++;              // inc $79
    } while (ram[0x79] != 0x04);  // lda $79 / cmp #$04 / bne @a1e7

    // State reset and transition
    ram[0x1704] = 0;              // stz $1704
    ram[0xAC] = 0;                // stz $ac
    ram[0x7B] = 0;                // stz $7b
    ram[0x1705] = 0x02;           // lda #$02 / sta $1705
    
    ram[0x171B] = ram[0x1701];    // lda $1701 / sta $171b
    
    // ldx $1706 / stx $1719 (X is 16-bit per field module convention)
    uint16_t x_val = read16(ram, 0x1706);
    write16(ram, 0x1719, x_val);

    play_map_song_emu(snes);     // jsr PlayMapSong
    ram[0x1A02]++;                // inc $1a02
}

// PITFALLS: 8 (Inherited mode A 8-bit for the counter loop and flag check)
// HELPERS: animate8_emu(snes), play_map_song_emu(snes), read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00A1=1, 0x1701=1, 0x1706=2
//   output_ram:  0x0079=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LandHover ($A1:DE)