#include "snes/snes.h"

// Implements a delay loop by waiting for VBlank `x` times.
// Entry: X = number of VBlanks to wait (16-bit)
// Uses $89 as temporary storage for the loop counter.
static void WaitSpecial_c(Snes *snes, uint16_t count) {
    uint8_t *ram = snes->ram;
    write16(ram, 0x89, count);          // stx $89
    do {
        wait_vblank_long_emu(snes);     // jsr WaitVblankLong
        uint16_t x = read16(ram, 0x89); // ldx $89
        x--;                            // dex
        write16(ram, 0x89, x);          // stx $89
    } while (x != 0);                   // bne @e1e0
}

// PITFALLS: 8 (X is 16-bit per `longi` module convention), 10 (goto label
// workaround not needed here)
// HELPERS: wait_vblank_long_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::WaitSpecial ($E1:DE)