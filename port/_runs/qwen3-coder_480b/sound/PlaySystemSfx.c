#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = sound effect index (8-bit)
// This function writes the sound effect index to hAPUIO0, waits for it
// to be acknowledged by reading it back, then clears hAPUIO0 and the
// first four bytes of zero page ($00-$03).
static void PlaySystemSfx_c(Snes *snes, uint8_t sfx_index) {
    uint8_t *ram = snes->ram;
    // sta hAPUIO0 - write sound effect index to APU I/O register 0
    ram[0x2140] = sfx_index;
    // cmp hAPUIO0 / bne - wait until value is acknowledged
    while (ram[0x2140] != sfx_index) {
        // spin wait
    }
    // stz hAPUIO0 - clear APU I/O register 0
    ram[0x2140] = 0;
    // stz $00-$03 - clear first four bytes of zero page
    ram[0x00] = 0;
    ram[0x01] = 0;
    ram[0x02] = 0;
    ram[0x03] = 0;
}

// PITFALLS: 1 (DB=$7E required for correct addressing of hAPUIO0)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: sound::PlaySystemSfx ($86:0008)