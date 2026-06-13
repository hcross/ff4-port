#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0, DP=0
// Purpose: Checks event switch $128A bit 6. If clear, sets graphics transfer 
// parameters in DP RAM and triggers Tfr3bppGfx.
static void _15c144_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $128a / and #$40 / bne @c162
    if ((ram[0x128A] & 0x40) != 0) {
        return;
    }

    // ldx #$5a00 / stx $4c
    write16(ram, 0x4C, 0x5A00);
    // ldx #$0600 / stx $4e
    write16(ram, 0x4E, 0x0600);
    // ldx #$9e00 / stx $4a
    write16(ram, 0x4A, 0x9E00);
    // lda #$1c / sta $49
    ram[0x49] = 0x1C;

    // jsl Tfr3bppGfx
    Tfr3bppGfx_emu(snes);
}

// PITFALLS: None applicable.
// HELPERS: Tfr3bppGfx_emu(snes) — delegates Tfr3bppGfx @ $00:860F
//          write16 — little-endian 16-bit accessors over ram[]
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x128A=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_15c144 ($C1:44)