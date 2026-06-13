#include "snes/snes.h"

// Entry mode: X 16-bit (xf=0), DB=$E1 (field module), DP=0
// Logic:
//   1. Save the current value of X (counter) into ram[0x89]
//   2. Loop until counter reach 0:
//      a. Call WaitVblankLong to synchronize with vertical blank
//      b. Decrement counter and store back to ram[0x89]
static void WaitSpecial_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // stx $89 (X is 16-bit)
    uint16_t count = snes->cpu->x;
    write16(ram, 0x89, count);

    do {
        // jsr WaitVblankLong (delegated)
        wait_vblank_long_emu(snes);

        // ldx $89 / dex / stx $89
        count = read16(ram, 0x89);
        count--;
        write16(ram, 0x89, count);

        // bne @e1e0
    } while (count != 0);
}

// PITFALLS: 1 (DB=$E1 for field module), 6 (X is 16-bit, ensuring 
// read16/write16 used for $89)
// HELPERS: wait_vblank_long_emu(snes) — delegates WaitVblankLong @ $00:912D
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  0x89=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE1
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::WaitSpecial ($E1:DE)