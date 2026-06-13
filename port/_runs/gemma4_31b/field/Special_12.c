#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine manages a timed loop that executes the event function _00dd58.
// It decrements a counter at $2C (clamped to 0x70) and a loop counter at $89.
static void Special_12_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Initialization
    write16(ram, 0x2C, 0x0100); // ldx #$0100 / stx $2c
    write16(ram, 0x2E, 0x0070); // ldx #$0070 / stx $2e
    write16(ram, 0x89, 0x00D0); // ldx #$00d0 / stx $89

    do {
        _00dd58_emu(snes);      // jsr _00dd58

        uint16_t x2c = read16(ram, 0x2C);
        x2c--;                  // dex
        write16(ram, 0x2C, x2c);

        // cpx #$0070 / bcs @dc2f
        // Pitfall 3: bcs branches when X >= 0x70. Reset to 0x70 only if X < 0x70.
        if (x2c < 0x0070) {
            write16(ram, 0x2C, 0x0070);
        }

        uint16_t x89 = read16(ram, 0x89);
        x89--;                  // dex
        write16(ram, 0x89, x89);

    } while (x89 != 0);          // bne @dc1d

    WaitVblankEvent_emu(snes);   // jmp WaitVblankEvent
}

// PITFALLS: 3 (CMP/BCS inversion: bcs branches when X >= $70, so we enter
//            the reset block only when X < $70)
// HELPERS: _00dd58_emu(snes), WaitVblankEvent_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x2C=2, 0x2E=2, 0x89=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_12 ($DC:0E)