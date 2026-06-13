#include "snes/snes.h"

// Delays for X vblank intervals. Stores X to $89, loops calling
// WaitVblankLong and decrementing $89 until zero.
static void WaitSpecial_c(Snes *snes, uint16_t count) {
    uint8_t *ram = snes->ram;
    write16(ram, 0x89, count);          // stx $89
    do {
        wait_vblank_long_emu(snes);     // jsr WaitVblankLong
        count = read16(ram, 0x89);      // ldx $89
        count = (uint16_t)(count - 1);  // dex (wraps 0→0xFFFF)
        write16(ram, 0x89, count);      // stx $89
    } while (count != 0);              // bne @e1e0
}

// PITFALLS: 1 (DB=$7E for stx/ldx absolute addressing)
// HELPERS: wait_vblank_long_emu(snes) — delegates WaitVblankLong @912D
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  (none beyond X register)
//   output_ram:  0x89=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::WaitSpecial ($E1:DE)