#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E1, DP=0
// This routine implements a countdown timer using register X,
// waiting for one Vblank period per decrement.
//   in : cpu->x = number of frames to wait
//   out: none (returns to caller)
static void WaitSpecial_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // stx $89 : store 16-bit X into WRAM (X is 16-bit per field module convention)
    uint16_t count = snes->cpu->x;
    write16(ram, 0x89, count);

    do {
        WaitVblankLong_emu(snes); // jsr WaitVblankLong
        
        // ldx $89 / dex / stx $89
        count = read16(ram, 0x89);
        count--;
        write16(ram, 0x89, count);
        
        // bne @e1e0 : loop until count is 0
    } while (count != 0);
}

// PITFALLS: 1 (DB=$E1), 6 (X is 16-bit, requires write16/read16 for $89)
// HELPERS: WaitVblankLong_emu(snes) — delegates WaitVblankLong @ 912D
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  0x89=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE1
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::WaitSpecial ($E1:DE)