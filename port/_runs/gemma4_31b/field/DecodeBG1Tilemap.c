#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FF, DP=0
// Logic:
//   Load the BG1 tilemap identifier from RAM and delegate the actual 
//   decoding process to DecodeSubTilemap.
static void DecodeBG1Tilemap_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $06f9
    uint8_t tilemap_id = ram[0x06F9];
    snes->cpu->a = tilemap_id;
    
    // Update Z/N flags to match LDA behavior for the callee
    snes->cpu->z = (tilemap_id == 0);
    snes->cpu->n = (tilemap_id & 0x80) != 0;

    // jsr DecodeSubTilemap
    decode_sub_tilemap_emu(snes);
}

// PITFALLS: None.
// HELPERS: decode_sub_tilemap_emu(snes) — delegates DecodeSubTilemap @ $FE:BA
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x06F9=1
//   output_ram: none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFF
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::DecodeBG1Tilemap ($FF:AB)