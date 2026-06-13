#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (implied), DP=0
// Logic:
//   Checks current map type from ram[0x1700].
//   If 0: calls TfrWaterGfx
//   If 1: calls TfrLavaGfx
//   Else: returns immediately.
static void TfrWaterLavaGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t map_type = ram[0x1700]; // lda $1700
    if (map_type != 0) {            // bne @8e5f
        tfr_water_gfx_emu(snes);   // jmp TfrWaterGfx
        return;
    }
    
    // This path is logically unreachable based on the ASM structure 
    // (@8e57 -> bne @8e5f -> jmp TfrWaterGfx), 
    // but the ASM has a branch to @8e5f if A != 0.
    // Correcting flow: 
    // if (map_type == 0) { jmp TfrWaterGfx; return; }
    // if (map_type == 1) { jmp TfrLavaGfx; return; }
    // else { rts; }
}

// Re-evaluating ASM flow for exact parity:
// @8e57: lda $1700
// @8e57: bne @8e5f  -> if A != 0, go to check for Lava
// @8e58: jmp TfrWaterGfx -> if A == 0, go to Water
// @8e5f: cmp #$01
// @8e60: bne @8e66  -> if A != 1, return (nothing to do)
// @8e61: jmp TfrLavaGfx -> if A == 1, go to Lava

static void TfrWaterLavaGfx_fixed_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t val = ram[0x1700];

    if (val == 0) {
        tfr_water_gfx_emu(snes);
        return;
    }
    if (val == 1) {
        tfr_lava_gfx_emu(snes);
        return;
    }
    // @8e66: rts
}

// PITFALLS: None relevant for this specific control-flow routine.
// HELPERS: tfr_water_gfx_emu(snes), tfr_lava_gfx_emu(snes)

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1700=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto