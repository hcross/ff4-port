#include "snes/snes.h"

// Logic:
// Copies four 16-bit words from the treasure data region ($7F48EE onwards)
// to the Direct Page (DP=0) buffer starting at $0700.
// Resets the accumulator to 8-bit mode before returning.
static void GetTreasureTiles_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // longa: A is 16-bit. 
    // Accesses are to $7Fxxxx, which are in the high WRAM/ROM mirror area.
    // The mapping $7F48EE corresponds to ram[0x148EE] in the 128KB WRAM 
    // context if we treat $7F:0000 as base 0x10000.
    
    write16(ram, 0x0700, read16(ram, 0x148EE)); // lda $7f48ee / sta $0700
    write16(ram, 0x0702, read16(ram, 0x149EE)); // lda $7f49ee / sta $0702
    write16(ram, 0x0704, read16(ram, 0x14AEE)); // lda $7f4aee / sta $0704
    write16(ram, 0x0706, read16(ram, 0x14BEE)); // lda $7fbee  / sta $0706

    // lda #$0000 / shorta
    snes->cpu->a = 0;
    snes->cpu->mf = true; 
}

// PITFALLS: 6 (Mode A 16-bit: explicit longa ensures 2-byte loads/stores).
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x148EE=2, 0x149EE=2, 0x14AEE=2, 0x14BEE=2
//   output_ram:  0x0700=2, 0x0702=2, 0x0704=2, 0x0706=2
//   entry_mode:  mf=auto, xf=auto, dp=0x0, db=0x7F
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetTreasureTiles ($9A:E9)