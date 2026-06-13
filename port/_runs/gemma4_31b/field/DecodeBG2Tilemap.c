#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FF, DP=0
// Logic:
//   Loads the BG2 tilemap index from 0x0FE3 and passes it to DecodeSubTilemap.
static void DecodeBG2Tilemap_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // lda $0fe3
    uint8_t tilemap_idx = ram[0x0FE3];
    snes->cpu->a = tilemap_idx;
    
    // jsr DecodeSubTilemap
    decode_sub_tilemap_emu(snes);
}

// PITFALLS: None (Simple wrapper for sub-routine)
// HELPERS: decode_sub_tilemap_emu(snes) — delegates DecodeSubTilemap @ $FF:B2 (actually $FF:B2 calls $FF:BA, but labeling provided is @feba)

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0FE3=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFF
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::DecodeBG2Tilemap ($FF:B2)