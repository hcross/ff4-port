#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0xAB (program bank), DP=0
// Logic:
//   Reads four 16-bit coordinates from the 7F bank (WRAM) and stores them
//   sequentially into DP memory (0x0700-0x0707).
//   Restores A to 8-bit mode before returning.
static void GetDoorTiles_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // longa: switch to 16-bit accumulator
    // lda $7f48de -> sta $0700
    uint16_t tile0 = read16(ram, 0x48DE); // WRAM bank $7F is mapped to snes->ram base
    write16(ram, 0x0700, tile0);

    // lda $7f49de -> sta $0702
    uint16_t tile1 = read16(ram, 0x49DE);
    write16(ram, 0x0702, tile1);

    // lda $7f4ade -> sta $0704
    uint16_t tile2 = read16(ram, 0x4ADE);
    write16(ram, 0x0704, tile2);

    // lda $7f4bde -> sta $0706
    uint16_t tile3 = read16(ram, 0x4BDE);
    write16(ram, 0x0706, tile3);

    // lda #0 : sets Z=1, N=0
    snes->cpu->a = 0;
    snes->cpu->z = true;
    snes->cpu->n = false;

    // shorta: Restore accumulator to 8-bit mode
    snes->cpu->mf = true;
}

// PITFALLS: 6 (longa ensures 16-bit loads/stores for the coordinates)
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x48DE=2, 0x49DE=2, 0x4ADE=2, 0x4BDE=2
//   output_ram:  0x0700=2, 0x0702=2, 0x0704=2, 0x0706=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xAB
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetDoorTiles ($AB:60)