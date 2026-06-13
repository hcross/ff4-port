#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DC, DP=0
// Purpose: A timed wait loop that calls a routine repeatedly.
// It decrements three counters ($89, $2C, $2E) until they reach thresholds,
// effectively creating a nested timing loop before jumping to WaitVblankEvent.
static void Special_13_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    write16(ram, 0x2C, 0x0070); // ldx #$0070 / stx $2c
    write16(ram, 0x2E, 0x0070); // ldx #$0070 / stx $2e
    write16(ram, 0x89, 0x00D0); // ldx #$00d0 / stx $89

loop_dc48:;
    _00dd58_emu(snes);          // jsr _00dd58

    uint16_t x89 = read16(ram, 0x89);
    x89--;                      // dex
    write16(ram, 0x89, x89);     // stx $89

    if (x89 >= 0x0080) {        // cpx #$0080 / bcs @dc48
        goto loop_dc48;
    }

    uint16_t x2c = read16(ram, 0x2C);
    x2c--;                      // dex
    write16(ram, 0x2C, x2c);     // stx $2c

    if (x2c != 0xFFF0) {         // cpx #$fff0 / bne @dc48
        goto loop_dc48;
    }

    wait_vblank_event_emu(snes); // jmp WaitVblankEvent
}

// PITFALLS: 3 (CMP/BCS inversion: bcs branches when X >= 0x80, so loop continues)
// HELPERS: _00dd58_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x2C=2, 0x2E=2, 0x89=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDC
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_13 ($DC:39)