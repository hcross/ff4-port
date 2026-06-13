#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$00, DP=0
// This routine resets sprites and initializes a coordinate/parameter block
// in WRAM before calling a secondary setup routine.
static void _00dd3a_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ResetSprites_emu(snes);           // jsr ResetSprites

    // ldx $2c / stx $0c (X 16-bit)
    uint16_t x1 = read16(ram, 0x2C);
    write16(ram, 0x0C, x1);

    // ldx $2e / stx $0e (X 16-bit)
    uint16_t x2 = read16(ram, 0x2E);
    write16(ram, 0x0E, x2);

    ram[0x91] = 0x18;                 // lda #$18 / sta $91
    ram[0x8F] = 0x78;                 // lda #$78 / sta $8f

    snes->cpu->y = 0x0180;            // ldy #$0180 (Y 16-bit)

    ram[0x92] = 0x60;                 // lda #$60 / sta $92

    _00dfc4_emu(snes);                // jsr _00dfc4
}

// PITFALLS: 1 (DB=$00), 6 (X/Y 16-bit, A 8-bit)
// HELPERS: ResetSprites_emu(snes), _00dfc4_emu(snes), read16, write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2C=2, 0x2E=2
//   output_ram:  0x0C=2, 0x0E=2, 0x8F=1, 0x91=1, 0x92=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00dd3a ($DD:3A)