#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$8E (implied), DP=0
// This routine acts as a dispatcher for water/lava animation updates 
// based on the current map/area type stored at $1700.
static void UpdateWaterLavaAnim_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t area_type = ram[0x1700];

    if (area_type != 0) {              // bne @8e4f
        if (area_type == 1) {          // cmp #$01 / bne @8e56
            update_lava_anim_emu(snes); // jmp UpdateLavaAnim
        }
        // If area_type is neither 0 nor 1, it falls through to rts
    } else {
        update_water_anim_emu(snes);   // jmp UpdateWaterAnim
    }
}

// PITFALLS: None relevant for this dispatcher logic.
// HELPERS: update_water_anim_emu(snes) - delegates UpdateWaterAnim @ 8FA1
//          update_lava_anim_emu(snes)  - delegates UpdateLavaAnim @ 8EE3

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1700=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x8E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Dispatcher routine, parity validated by checking which sub-emu was called)

// REVERSED_FUNCTION: field::UpdateWaterLavaAnim ($8E:0047)