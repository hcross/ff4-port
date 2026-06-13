#include "snes/snes.h"

// WaitSpecial: delay loop that calls WaitVblankLong X times.
// X (16-bit) is the iteration count.  Uses $89-$8A as a temporary
// down-counter.  At exit X=0 and $89-$8A=0.
//
// Entry mode: A 8-bit (mf=true), X/Y 16-bit (xf=false), DB=$7E, DP=0.
// No flags are consulted on entry (first instruction is stx, not a branch).
static void WaitSpecial_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t *ram = snes->ram;

    // stx $89  – store initial 16-bit count
    write16(ram, 0x89, cpu->x);

    // Loop: jsr WaitVblankLong / ldx $89 / dex / stx $89 / bne
    while (1) {
        wait_vblank_long_emu(snes);               // jsr WaitVblankLong
        uint16_t x = read16(ram, 0x89);           // ldx $89
        x--;                                      // dex
        write16(ram, 0x89, x);                    // stx $89
        cpu->x = x;                               // keep CPU X in sync
        if (x == 0) break;                        // bne @e1e0 (exit when zero)
    }
    // At this point X=0, $89-$8A=0 (matches asm state after rts)
}

// PITFALLS:
//   1 (DB=$7E required – absolute $89 must hit WRAM, not a hardware register)
//   6 (mode X 16-bit assumed; if the caller had xf=true the asm would
//      behave as an 8-bit loop and parity would fail)
// HELPERS:
//   wait_vblank_long_emu(snes) – delegates WaitVblankLong @ $E1:912D
// CONTRACT:
//   inputs_reg:  x=16 (count)
//   inputs_ram:  none
//   output_ram:  0x89=2 (zeroed after loop)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
// REVERSED_FUNCTION: field::WaitSpecial ($E1:DE)